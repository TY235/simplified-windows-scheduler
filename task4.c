#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"


void * producer(){

}

void * consumer(){

}




// void createProcess(){
//     struct process * A[52];
//     // printf("%d",sizeof(A)/sizeof(A[0]));
//     for(int i = 0; i < 1; i++){

//     }
// }




int main(int argc, char** argv){

    
    sem_init(&delayProducer, 0, 0);		
	sem_init(&delayConsumer, 0, 0);
	sem_init(&sync, 0, 1);
    

    pthread_t prod, cons1, cons2;

    pthread_create(&prod, NULL, producer, NULL);
	pthread_create(&cons1, NULL, consumer, NULL);
	pthread_create(&cons2, NULL, consumer, NULL);



    return 0;
}
