#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"

sem_t empty;
// sem_t full;
sem_t sync;

double dAverageResponseTime = 0;
double dAverageTurnAroundTime = 0;
int totalJobsConsumed = 0;

struct linkedList {

    int currentBufferSize;
    sem_t bufferSync;
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
        sem_wait(&sync);
		dAverageResponseTime += iResponseTime;
        sem_post(&sync);
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2	 ? "FCFS" : "RR",pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iResponseTime);
		return pProcess;
	} 
    else if(pProcess->iPreviousBurstTime == pProcess->iInitialBurstTime && pProcess->iRemainingBurstTime == 0)
	{
		iResponseTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oStartTime);	
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess->oTimeCreated, oEndTime);
        sem_wait(&sync);
		dAverageResponseTime += iResponseTime;
		dAverageTurnAroundTime += iTurnAroundTime;
        sem_post(&sync);
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
        sem_wait(&sync);
		dAverageTurnAroundTime += iTurnAroundTime;
        sem_post(&sync);
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iPreviousBurstTime, pProcess->iRemainingBurstTime, iTurnAroundTime);
		free(pProcess);
		return NULL;
	}
}

// Consumer function
// void *consumer(void* consumerID){

//     int consID = *((int *)consumerID); /*Typecast void pointer to integer*/
//     struct process * pProcess;
//     struct process * remainingProcess;
//     while(1){
//         for(int i = 0; i < MAX_PRIORITY; i++){
//             // sem_wait(&full);
//             sem_wait(&buffer[i].bufferSync);
//             if(buffer[i].currentBufferSize == 0){
//                 sem_post(&buffer[i].bufferSync);
//                 break;
//             }
//             // Critical Section
//             buffer[i].currentBufferSize--;
//             pProcess = removeFirst(&buffer[i].head,&buffer[i].tail);
//             sem_post(&buffer[i].bufferSync);
//             // End Section

//             runJob(pProcess,&oStartTime,&oEndTime);
//             remainingProcess = processJob(consID,pProcess,oStartTime,oEndTime);
            
//             if(remainingProcess == NULL){
//                 sem_wait(&sync);
//                 totalJobsConsumed++;
//                 sem_post(&sync);
//                 sem_post(&empty);
//             }
//             else{
//                 sem_wait(&buffer[i].bufferSync);
//                 buffer[i].currentBufferSize++;
//                 addLast(remainingProcess,&buffer[remainingProcess->iPriority].head,&buffer[remainingProcess->iPriority].tail);
//                 sem_post(&buffer[i].bufferSync);
          
//             }

//             sem_wait(&sync);
//             if (totalJobsConsumed==MAX_NUMBER_OF_JOBS){
//                 c = 1;    
//                 sem_post(&sync);
//                 break;
//             }      
//             sem_post(&sync);
//         }
      
//         if (c==1){
//             break;
//         }
//     }
// }


// void *consumer(void* consumerID){

//     int consID = *((int *)consumerID); /*Typecast void pointer to integer*/
//     int c = 0;
  
//     struct process * pProcess;
//     struct process * remainingProcess;

//     while(1){
//         for(int i = 0; i < MAX_PRIORITY; i++){
//             // sem_wait(&full);
//             if(buffer[i].currentBufferSize != 0){
//                 sem_wait(&buffer[i].bufferSync);
//                 // Critical Section
//                 buffer[i].currentBufferSize--;
//                 pProcess = removeFirst(&buffer[i].head,&buffer[i].tail);
//                 sem_post(&buffer[i].bufferSync);
//                 // End Section
            
//                 struct timeval oStartTime, oEndTime; 
//                 runJob(pProcess,&oStartTime,&oEndTime);
//                 remainingProcess = processJob(consID,pProcess,oStartTime,oEndTime);
                
//                 if(remainingProcess == NULL){
//                     sem_wait(&sync);
//                     totalJobsConsumed++;
//                     sem_post(&sync);
//                     sem_post(&empty);
//                 }
//                 else{
             
//                     sem_wait(&buffer[remainingProcess->iPriority].bufferSync);
//                     addLast(remainingProcess,&buffer[remainingProcess->iPriority].head,&buffer[remainingProcess->iPriority].tail);
//                     buffer[remainingProcess->iPriority].currentBufferSize++;
//                     sem_post(&buffer[remainingProcess->iPriority].bufferSync);
            
//                 }
//                 // break;
//             }
//             // sem_post(&buffer[i].bufferSync);

//             sem_wait(&sync);
//             if (totalJobsConsumed==NUMBER_OF_JOBS){
//                 c = 1;    
//                 sem_post(&sync);
//                 break;
//             }      
//             sem_post(&sync);
//         }
      
//         if (c){
//             break;
//         }
//     }
    
// }


void *consumer(void* consumerID){
    
    int consID = *((int *)consumerID); /*Typecast void pointer to integer*/
    int c = 0;
   
    struct process * pProcess;
    struct process * remainingProcess;

    while(1){
        for(int i = 0; i < MAX_PRIORITY; i++){
            if(buffer[i].currentBufferSize != 0){
                sem_wait(&buffer[i].bufferSync);
                buffer[i].currentBufferSize--;
                pProcess = removeFirst(&buffer[i].head,&buffer[i].tail);
                sem_post(&buffer[i].bufferSync);   

                struct timeval oStartTime, oEndTime;
                runJob(pProcess,&oStartTime,&oEndTime);
                remainingProcess = processJob(consID,pProcess,oStartTime,oEndTime);
            
                if(remainingProcess==NULL){
                    sem_wait(&sync);
                    totalJobsConsumed++;
                    sem_post(&sync);
                    sem_post(&empty);
                }
                else{
                    sem_wait(&buffer[remainingProcess->iPriority].bufferSync);
                    addLast(remainingProcess,&buffer[remainingProcess->iPriority].head,&buffer[remainingProcess->iPriority].tail);
                    buffer[remainingProcess->iPriority].currentBufferSize++;
                    sem_post(&buffer[remainingProcess->iPriority].bufferSync);
                }
            }
          
            // sem_post(&buffer[i].bufferSync);

            sem_wait(&sync);
            if (totalJobsConsumed==NUMBER_OF_JOBS){
                c = 1;    
                sem_post(&sync);
                break;
            }      
            sem_post(&sync);
        }
        if (c){
            break;
        }
    }
}


// Producer function
void *producer(){

    // Create the number of jobs defined in coursework.h
    int i;
    while(i < NUMBER_OF_JOBS){    
        // Use counting semaphore to avoid over-filled the linked list
        sem_wait(&empty);
        struct process * pProcess = generateProcess();
        // Enter critical section of the respective linked list
        sem_wait(&buffer[pProcess->iPriority].bufferSync);
        addLast(pProcess,&buffer[pProcess->iPriority].head,&buffer[pProcess->iPriority].tail);
        buffer[pProcess->iPriority].currentBufferSize++;
        // Leave critical section of the respective linked list
        sem_post(&buffer[pProcess->iPriority].bufferSync);
        printf("Producer 0, Process Id = %d (%s), Priority = %d, Initial Burst Time = %d\n", pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iInitialBurstTime);
        i++;
    }
    
}

int main(){

    int i = 0;
    while( i < MAX_PRIORITY){
        buffer[i].currentBufferSize = 0;
        sem_init(&buffer[i].bufferSync,0,1);
        buffer[i].head = NULL;
        buffer[i].tail = NULL;
        i++;
    }

    sem_init(&empty, 0, MAX_BUFFER_SIZE);
    // sem_init(&full, 0, 0);
    sem_init(&sync, 0, 1);

    int consumerID[NUMBER_OF_CONSUMERS];
    pthread_t prod, cons[NUMBER_OF_CONSUMERS];

    // Create the threads
    // Create the number of consumer threads defined in coursework.h
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
    dAverageResponseTime /= NUMBER_OF_JOBS;
	dAverageTurnAroundTime /= NUMBER_OF_JOBS;
    
    printf("Average Response Time = %.6f\n",dAverageResponseTime);
    printf("Average Turnaround Time = %.6f\n",dAverageTurnAroundTime);

    return 0;
    
}
