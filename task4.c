#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"


void * producer(){

    while(1){
        sem_wait(&empty);
        sem_wait(&sync);
        
        sem_post(&sync);
        sem_post(&full);
    }
}

void * consumer(){

     while(1){
        sem_wait(&empty);
        sem_wait(&sync);

        sem_post(&sync);
        sem_post(&full);
    }
}




// void createProcess(){
//     struct process * A[52];
//     // printf("%d",sizeof(A)/sizeof(A[0]));
//     for(int i = 0; i < 1; i++){

//     }
// }




int main(int argc, char** argv){
    
    sem_init(&full, 0, 0);		
	sem_init(&empty, 0, MAX_BUFFER_SIZE);
	sem_init(&sync, 0, 1);

    pthread_t prod, cons1, cons2;

    pthread_create(&prod, NULL, producer, NULL);
	pthread_create(&cons1, NULL, consumer, NULL);
	pthread_create(&cons2, NULL, consumer, NULL);

    pthread_join(prod, NULL);
	pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    return 0;
    
    }
