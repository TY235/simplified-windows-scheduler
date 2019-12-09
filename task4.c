#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"

sem_t noOfJobs;
struct timeval oStartTime;
struct timeval oEndTime; 
int dAverageResponseTime = 0;
int dAverageTurnAroundTime = 0;

struct linkedList {

    int currentBufferSize;
    sem_t sync;
    struct element * head;
    struct element * tail;

} buffer[MAX_PRIORITY];

struct process * processJob(int iConsumerId, struct process * pProcess, struct timeval oStartTime, struct timeval oEndTime)
{
	int iResponseTime;
	int iTurnAroundTime;
	
    if(pProcess->iPreviousBurstTime == pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime > 0)
	{
		iResponseTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oStartTime);	
		dAverageResponseTime += iResponseTime;
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2	 ? "FCFS" : "RR",pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iResponseTime);
		return pProcess;
	} 
    else if(pProcess->iPreviousBurstTime == pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime == 0)
	{
		iResponseTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oStartTime);	
		dAverageResponseTime += iResponseTime;
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oEndTime);
		dAverageTurnAroundTime += iTurnAroundTime;
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iResponseTime, iTurnAroundTime);
		free(pProcess);
		return NULL;
	} 
    else if(pProcess->iPreviousBurstTime != pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime > 0)
	{
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime);
		return pProcess;
	} 
    else if(pProcess->iPreviousBurstTime != pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime == 0)
	{
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oEndTime);
		dAverageTurnAroundTime += iTurnAroundTime;
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iTurnAroundTime);
		free(pProcess);
		return NULL;
	}
}

// Consumer function
void *consumer(void* consumerID){
    int consID = *((int *)consumerID);
    struct process * pProcess;
    struct process * remainingProcess;
    while(1){
        for(int i = 0; i < MAX_PRIORITY; i++){
            if(buffer[i].currentBufferSize == 0){
               continue;
            }
            sem_wait(&buffer[i].sync);    
            pProcess = removeFirst(&buffer[i].head,&buffer[i].tail);
            sem_post(&buffer[i].sync);
            runJob(pProcess,&oStartTime,&oEndTime);
            remainingProcess = processJob(consID,pProcess,oStartTime,oEndTime);
            sem_post(&noOfJobs);

            if(remainingProcess == NULL){
                
            }
            else{
                
            }
            

        }
    }


}


// Producer function
void *producer(){

    // Create the number of jobs defined in coursework.h
    for(int i; i < NUMBER_OF_JOBS; i++){
        
        // Use counting semaphore to count the number of jobs left to do
        sem_wait(&noOfJobs);
        struct process * pProcess = generateProcess();
        // Ignore the current process if respective buffer size is more than 100
        if(buffer[pProcess->iPriority].currentBufferSize > MAX_BUFFER_SIZE){
            i--;
            free(pProcess);
            continue;
        } 
        // Enter critical section of the respective linked list
        sem_wait(&buffer[pProcess->iPriority].sync);
        addLast(pProcess,&buffer[pProcess->iPriority].head,&buffer[pProcess->iPriority].tail);
        buffer[pProcess->iPriority].currentBufferSize++;
        // Leave critical section of the respective linked list
        sem_post(&buffer[pProcess->iPriority].sync);
        printf("Producer 0, Process Id = %d (%s), Priority = %d, Initial Burst Time = %d\n", pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iInitialBurstTime);
    
    }
}

int main(){


    for(int i = 0; i < MAX_PRIORITY; i++){
        buffer[i].currentBufferSize = 0;
        sem_init(&buffer[i].sync,0,1);
        buffer[i].head = NULL;
        buffer[i].tail = NULL;
    }

    sem_init(&noOfJobs, 0, MAX_BUFFER_SIZE);

    int consumerID[NUMBER_OF_CONSUMERS];
    pthread_t prod, cons[NUMBER_OF_CONSUMERS];

    // Create the threads
    // Create the number of consumer threads defined in coursework.h
    for(int j = 0; j < NUMBER_OF_CONSUMERS; j++){
        consumerID[j] = j;
        pthread_create(&cons[j],NULL,consumer,(void *)&consumerID[j]);
    }
    
    // Create one producer thread
    pthread_create(&prod, NULL, producer, NULL);

    // Join all the consumer threads
    for(int j = 0; j < NUMBER_OF_CONSUMERS; j++){
        pthread_join(cons[j],NULL);
    }

    // Join the only producer thread
    pthread_join(prod,NULL);

    return 0;
    
}
