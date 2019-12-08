#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"

sem_t noOfJobs;
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

void * consumer(consumerID){



}

void * producer(){
    for(int i; i < NUMBER_OF_JOBS; i++){
        sem_wait(&noOfJobs);
        struct process * oTemp = generateProcess();
        sem_wait(&buffer[i].sync);
        addLast(oTemp,&buffer[i].head,&buffer[i].tail);
        printf("Producer 0, Process Id = %d (%s), Priority = %d, Initial Burst Time %d", pProcess->iProcessId, pProcess->iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess->iPriority, pProcess->iInitialBurstTime);
        sem_post(&buffer[i].sync);
    }
}

int main(){

    int
    sem_init(&numofJobs, 0, MAX_BUFFER_SIZE);
    
    for(int i = 0; i < MAX_PRIORITY; i++){
        buffer[i].currentBufferSize = 0;
        sem_init(&buffer[i].sync,0,0);
        buffer[i].head = NULL;
        buffer[i].tail = NULL;
    }

    int consumerID[NUMBER_OF_CONSUMERS];
    pthread_t producer, consumer[NUMBER_OF_CONSUMERS];

    // Create the threads
    for(int j = 0; j < NUMBER_OF_CONSUMERS; j++){
        consumerID = j;
        pthread_create(&consumer[j],NULL,consumer,(void *)consumerID[j]);
    }
    
    pthread_create(&producer, NULL, producer, NULL);

    for(int j = 0; j < NUMBER_OF_CONSUMERS; j++){
        pthread_join(consumer[j],NULL);
    }

    pthread_join(producer,NULL);

    return 0;
    
}
