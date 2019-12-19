#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"


// Declarations
sem_t empty;
sem_t sync;

double dAverageResponseTime = 0;
double dAverageTurnAroundTime = 0;
int totalJobsConsumed = 0;

// Structure of the buffer
struct linkedList {

    int currentBufferSize;
    sem_t bufferSync;
    struct element * head;
    struct element * tail;

} buffer[MAX_PRIORITY];


// To check if a process has completed
struct process * processJob(int iConsumerId, struct process * pProcess, struct timeval oStartTime, struct timeval oEndTime)
{
	int iResponseTime;
	int iTurnAroundTime;
	
    // Process that has not completed in the first run
    if(pProcess->iPreviousBurstTime == pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime > 0)
	{
		iResponseTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oStartTime);	
        // Enters critical section when changing the response time
        sem_wait(&sync);
		dAverageResponseTime += iResponseTime;
        sem_post(&sync);
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2	 ? "FCFS" : "RR",pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iResponseTime);
		return pProcess;
	} 
    // Process that had completed in the first run
    else if(pProcess->iPreviousBurstTime == pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime == 0)
	{
		iResponseTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oStartTime);	
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oEndTime);
        // Enters critical section when changing the response time and turnaroud time
        sem_wait(&sync);
		dAverageResponseTime += iResponseTime;
		dAverageTurnAroundTime += iTurnAroundTime;
        sem_post(&sync);
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iResponseTime, iTurnAroundTime);
		free(pProcess);
		return NULL;
	} 
    // Process that has not completed in the second or above run
    else if(pProcess->iPreviousBurstTime != pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime > 0)
	{
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime);
		return pProcess;
	} 
    // Process that had completed in the second or above run
    else if(pProcess->iPreviousBurstTime != pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime == 0)
	{   
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oEndTime);
        // Enters critical section when changing the turnaround time
        sem_wait(&sync);
		dAverageTurnAroundTime += iTurnAroundTime;
        sem_post(&sync);
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iTurnAroundTime);
		free(pProcess);
		return NULL;
	}
}


// Consumer function
void *consumer(void* consumerID){
    
    // Typecast void pointer to integer
    int consID = *((int *)consumerID);
    // Check if all the jobs has completed
    bool completed = false;
   
    struct process * pProcess;
    struct process * remainingProcess;

    // Keeps running until all the jobs has completed
    while(1){
        // Runs from the top priority buffer to the lowest priority buffer
        for(int i = 0; i < MAX_PRIORITY; i++){

            // Process jobs when the buffer is not empty
            if(buffer[i].currentBufferSize != 0){
                // Enter critical section
                sem_wait(&buffer[i].bufferSync);
                // Update the buffer size
                buffer[i].currentBufferSize--;
                // Remove the first job of the buffer 
                pProcess = removeFirst(&buffer[i].head,&buffer[i].tail);
                sem_post(&buffer[i].bufferSync);

                // Initialize the the current time
                struct timeval oStartTime, oEndTime;
                // Simulate the running of the process
                runJob(pProcess,&oStartTime,&oEndTime);
                // Call processJob function to calculate the responseTime and turnAroundTime
                remainingProcess = processJob(consID,pProcess,oStartTime,oEndTime);
            
                // Update the total jobs consumed by the consumer if process has completed
                if(remainingProcess==NULL){
                    // Enter critical section when updating the value
                    sem_wait(&sync);
                    totalJobsConsumed++;
                    sem_post(&sync);
                    // Update the counting semaphore
                    sem_post(&empty);
                }
                else{
                    // Enter the critical section of the remainingProcess buffer
                    sem_wait(&buffer[remainingProcess->iPriority].bufferSync);
                    // Add the incomplete job to the end of its respective buffer
                    addLast(remainingProcess,&buffer[remainingProcess->iPriority].head,&buffer[remainingProcess->iPriority].tail);
                    // Update the buffer size after adding in the incomplete job to the list
                    buffer[remainingProcess->iPriority].currentBufferSize++;
                    sem_post(&buffer[remainingProcess->iPriority].bufferSync);
                }
            }
            
            // If the all the jobs produced are consumed
            if (totalJobsConsumed==NUMBER_OF_JOBS){
                sem_wait(&sync);
                // Set a flag for exiting while loop
                completed = true;    
                sem_post(&sync);
                // Break from the for loop
                break;
            }      
        }
        // Break from the while loop if everything is completed
        if (completed){
            break;
        }
    }
}


// Producer function
void *producer(){

    // Create the number of jobs defined in coursework.h
    int i;
    while(i < NUMBER_OF_JOBS){    
        // Use counting semaphore to avoid over-filling the buffer
        sem_wait(&empty);
        // Generate process
        struct process * pProcess = generateProcess();
        // Enter critical section of the respective buffer
        sem_wait(&buffer[pProcess->iPriority].bufferSync);
        // Add the process to the end of the respective buffer
        addLast(pProcess,&buffer[pProcess->iPriority].head,&buffer[pProcess->iPriority].tail);
        // Update the buffer size
        buffer[pProcess->iPriority].currentBufferSize++;
        // Leave critical section of the respective buffer
        sem_post(&buffer[pProcess->iPriority].bufferSync);
        printf("Producer 0, Process Id = %d (%s), Priority = %d, Initial Burst Time = %d\n", pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iInitialBurstTime);
        i++;
    }
    
}

int main(){

    int i = 0;
    // Create 32 buffers for 32 priorities
    // Initialize the 32 buffers
    while( i < MAX_PRIORITY){
        buffer[i].currentBufferSize = 0;
        sem_init(&buffer[i].bufferSync,0,1);
        buffer[i].head = NULL;
        buffer[i].tail = NULL;
        i++;
    }

    // Initiaize the semaphores
    // 1 counting semaphore sets to the buffer size to avoid over filling the buffers 
    sem_init(&empty, 0, MAX_BUFFER_SIZE);
    // 1 binary semaphores for updating values that are shared among the buffers
    sem_init(&sync, 0, 1);

    // Create an array of consumers stated in the 'coursework.h' file
    int consumerID[NUMBER_OF_CONSUMERS];
    // Create a producer thread and a numbers of consumer threads
    pthread_t prod, cons[NUMBER_OF_CONSUMERS];

    // Create the threads
    // Create the number of consumer threads defined in 'coursework.h' file
    int j = 0;
    while(j < NUMBER_OF_CONSUMERS){
        consumerID[j] = j;
        pthread_create(&cons[j],NULL,consumer,(void *)&consumerID[j]);
         j++;
    }
    
    // Create one producer thread
    pthread_create(&prod, NULL, producer, NULL);

    // Join all the consumer threads
    int k = 0;
    while(k < NUMBER_OF_CONSUMERS){
        pthread_join(cons[k],NULL);
         k++;
    }

    // Join the only producer thread
    pthread_join(prod,NULL);

    // Calculate the average of response time and turnaround time
    dAverageResponseTime /= NUMBER_OF_JOBS;
	dAverageTurnAroundTime /= NUMBER_OF_JOBS;
    
    printf("Average Response Time = %.6f\n",dAverageResponseTime);
    printf("Average Turnaround Time = %.6f\n",dAverageTurnAroundTime);

    return 0;
    
}
