#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"

// Declare the pointers required for linked list
struct element *pHead = NULL;
struct element *pTail = NULL;
struct element *pPointer = NULL; //Current pointer

struct timeval oStartTime;
struct timeval oEndTime; 

struct process * oTemp = NULL;

double avgResponseTime = 0;
double avgTurnAroundTime = 0;
int responseTime = 0;
int turnAroundTime = 0;


// // Producer thread function
void * producer(){

    
    // Display and add all processes to the linked list
	for(int i = 0; i < NUMBER_OF_JOBS; i++){
		oTemp = generateProcess();
		addLast(oTemp,&pHead,&pTail);
	}	


    while(1){
        break;
        // sem_wait(&empty);
        // sem_wait(&sync);
        
        // sem_post(&sync);
        // sem_post(&full);
    }
}

// Consumer threads function
void * consumer(){

     while(1){

         
        // Run the linked list and calculate the response time and turnaround time
        for(int j = 0; j < NUMBER_OF_JOBS; j++){
            runNonPreemptiveJob((struct process*)pPointer->pData, &oStartTime, &oEndTime);	
            responseTime = getDifferenceInMilliSeconds(((struct process*)(pPointer->pData))->oTimeCreated,oStartTime);
            turnAroundTime = getDifferenceInMilliSeconds(((struct process*)(pPointer->pData))->oTimeCreated,oEndTime);
            avgResponseTime += responseTime;
            avgTurnAroundTime += turnAroundTime; 
            printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %d, Turn Around Time = %d\n",((struct process*)(pPointer->pData))->iProcessId,((struct process*)(pPointer->pData))->iInitialBurstTime,((struct process*)(pPointer->pData))->iRemainingBurstTime,responseTime,turnAroundTime);
            pPointer = pPointer->pNext; 
    	}

         break;
        // sem_wait(&empty);
        // sem_wait(&sync);

        // sem_post(&sync);
        // sem_post(&full);
    }
}




// void createProcess(){
//     struct process * A[52];
//     // printf("%d",sizeof(A)/sizeof(A[0]));
//     for(int i = 0; i < 1; i++){

//     }
// }







int main(int argc, char** argv){


    // Initialize 2 counting semaphores and 1 binary semaphore
    // sem_init(&full, 0, 0);		
	// sem_init(&empty, 0, MAX_BUFFER_SIZE);
	// sem_init(&sync, 0, 1);

    pthread_t prod, cons1, cons2;

    // Create 1 producer and 2 consumer threads
    pthread_create(&prod, NULL, producer, NULL);
	pthread_create(&cons1, NULL, consumer, NULL);
	pthread_create(&cons2, NULL, consumer, NULL);

    // Join  the threads 
    pthread_join(prod, NULL);
	pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);
    



    return 0;
    
}

