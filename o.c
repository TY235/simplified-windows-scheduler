#include "coursework.h"
#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Counting semaphore representing the number of slots for jobs currently in the system.
sem_t slots;
sem_t sync_global;

struct buffer {
	// Head and tail pointers of the buffer.
	struct element * head;
	struct element * tail;
	// Number of processes in the buffer.
	int count;
	// The binary semaphore for synchronising access to the buffer.
	sem_t sync;
};

// Priority queues.
struct buffer buffers[MAX_PRIORITY];

// Number of jobs consumed in total.
int numConsumed;

double dAverageResponseTime, dAverageTurnAroundTime;

struct process * processJob(int iConsumerId, struct process * pProcess, struct timeval oStartTime, struct timeval oEndTime) {
	int iResponseTime;
	int iTurnAroundTime;
	if (pProcess -> iPreviousBurstTime == pProcess -> iInitialBurstTime && pProcess -> iRemainingBurstTime > 0) {
		// A process running the first time did not finish.
		iResponseTime = getDifferenceInMilliSeconds(pProcess -> oTimeCreated, oStartTime);
		sem_wait(&sync_global);
		dAverageResponseTime += iResponseTime;
		sem_post(&sync_global);
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d\n", iConsumerId, pProcess -> iProcessId, pProcess -> iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess -> iPriority, pProcess -> iPreviousBurstTime, pProcess -> iRemainingBurstTime, iResponseTime);
	} else if (pProcess -> iPreviousBurstTime == pProcess -> iInitialBurstTime && pProcess -> iRemainingBurstTime == 0)	{
		// A process running the first time finished.
		iResponseTime = getDifferenceInMilliSeconds(pProcess -> oTimeCreated, oStartTime);
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess -> oTimeCreated, oEndTime);
		sem_wait(&sync_global);
		dAverageResponseTime += iResponseTime;
		dAverageTurnAroundTime += iTurnAroundTime;
		sem_post(&sync_global);
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess -> iProcessId, pProcess -> iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess -> iPriority, pProcess -> iPreviousBurstTime, pProcess -> iRemainingBurstTime, iResponseTime, iTurnAroundTime);
		free(pProcess);
		pProcess = NULL;
	} else if (pProcess -> iPreviousBurstTime != pProcess -> iInitialBurstTime && pProcess -> iRemainingBurstTime > 0) {
		// A process not running the first time did not finish.
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n", iConsumerId, pProcess -> iProcessId, pProcess -> iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess -> iPriority, pProcess -> iPreviousBurstTime, pProcess -> iRemainingBurstTime);
	} else if (pProcess -> iPreviousBurstTime != pProcess -> iInitialBurstTime && pProcess -> iRemainingBurstTime == 0)	{
		// A process not running the first time finished.
		iTurnAroundTime = getDifferenceInMilliSeconds(pProcess -> oTimeCreated, oEndTime);
		sem_wait(&sync_global);
		dAverageTurnAroundTime += iTurnAroundTime;
		sem_post(&sync_global);
		printf("Consumer %d, Process Id = %d (%s), Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turnaround Time = %d\n", iConsumerId, pProcess -> iProcessId, pProcess -> iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", pProcess -> iPriority, pProcess -> iPreviousBurstTime, pProcess -> iRemainingBurstTime, iTurnAroundTime);
		free(pProcess);
		pProcess = NULL;
	}
	return pProcess;
}

void * producer() {
	for (int i = 0; i < NUMBER_OF_JOBS; i++) {
		// Go to sleep if there is currently no slot for jobs in the system.
		// Otherwise decrement the number of slots.
        printf("asd");
		sem_wait(&slots);
		printf("zxc");
		// Create a new process.
		struct process * p = generateProcess();
		
		// Put process in the buffer.
		sem_wait(&buffers[p -> iPriority].sync);
		addLast(p, &buffers[p -> iPriority].head, &buffers[p -> iPriority].tail);
		// Increment the number of jobs in the buffer.
		buffers[p -> iPriority].count++;
		// Print properties of new process.
		printf("Producer 0, Process Id = %d (%s), Priority = %d, Initial Burst Time %d\n", p -> iProcessId, p -> iPriority < MAX_PRIORITY / 2 ? "FCFS" : "RR", p -> iPriority, p -> iInitialBurstTime);
		sem_post(&buffers[p -> iPriority].sync);
        printf("qwe");
	}
	pthread_exit(NULL);
}

void * consumer(void * id) {
	int consumerID = * ((int *) id);
	int consumer_finished = 0;
	while (1) {
		for (int i = 0; i < MAX_PRIORITY; i++) {
			sem_wait(&buffers[i].sync);
			if (buffers[i].count != 0) {
				// The first non-empty buffer.
				// Update the number of full cells.
				buffers[i].count--;
				// Take first process in the buffer.
				struct process * p = removeFirst(&buffers[i].head, &buffers[i].tail);
				sem_post(&buffers[i].sync);
				
				struct timeval pStartTime, pEndTime;
				// Run the job.
				runJob(p, &pStartTime, &pEndTime);
				// Time calculations.
				struct process * pProcessed = processJob(consumerID, p, pStartTime, pEndTime);
				
				// Check if the process has finished.
				if (pProcessed != NULL) {
					// Reschedule the process.
					sem_wait(&buffers[i].sync);
					addLast(pProcessed, &buffers[i].head, &buffers[i].tail);
					buffers[i].count++;
					sem_post(&buffers[i].sync);
				} else {
					// Update the number of consumed jobs.
					sem_wait(&sync_global);
					numConsumed++;
					sem_post(&sync_global);
					// Update the number of slots for jobs currently in the system.
					sem_post(&slots);
				}
				// Go back to the highest priority level.
				break;
			}
			sem_post(&buffers[i].sync);
			
			// Check whether all jobs have been consumed.
			sem_wait(&sync_global);
			if (numConsumed == NUMBER_OF_JOBS) {
				consumer_finished = 1;
				sem_post(&sync_global);
				break;
			}
			sem_post(&sync_global);
		}
		if (consumer_finished) {
			break;
		}
	}
	pthread_exit(NULL);
}

int main() {
	// Initialise global semaphores.
	sem_init(&slots, 0, MAX_BUFFER_SIZE);
	sem_init(&sync_global, 0, 1);
	
	// Initialise buffers.
	for (int i = 0; i < MAX_PRIORITY; i++) {
		// Initialise head and tail pointers.
		buffers[i].head = NULL;
		buffers[i].tail = NULL;
		// Initialise number of processes in the buffer.
		buffers[i].count = 0;
		// Initialise synchronisation semaphore.
		sem_init(&buffers[i].sync, 0, 1);
	}
	
	// Create the producer thread.
	pthread_t tProducer;
	pthread_create(&tProducer, NULL, producer, NULL);
	
	// Create multiple consumer threads.
	pthread_t tConsumers[NUMBER_OF_CONSUMERS];
	int consumerIDs[NUMBER_OF_CONSUMERS];
	// Use a fixed array of thread IDs instead of an incrementing temporary value as thread arguments.
	// Otherwise the threads will refer to a changing or invalid value in the main thread.
	for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
		consumerIDs[i] = i;
	}
	for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
		pthread_create(&tConsumers[i], NULL, consumer, (void *) &consumerIDs[i]);
	}
	
	// Tell the main thread to wait for all threads to finish.
	pthread_join(tProducer, NULL);
	for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
		pthread_join(tConsumers[i], NULL);
	}
	
	// Print average response time and average turn around time.
	dAverageResponseTime /= NUMBER_OF_JOBS;
	dAverageTurnAroundTime /= NUMBER_OF_JOBS;
	printf("Average Response Time = %lf\n", dAverageResponseTime);
	printf("Average Turn Around Time = %lf\n", dAverageTurnAroundTime);
	
	return 0;
}