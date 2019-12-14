void * consumer(void * id) {
	int consumerID = * ((int *) id);
	int consumer_finished = 0;
	while (1) {
		for (int i = 0; i < MAX_PRIORITY; i++) {
			sem_wait(&buffer[i].bufferSync);
			if (buffer[i].currentBufferSize != 0) {
				// The first non-empty buffer.
				// Update the number of full cells.
				buffer[i].currentBufferSize--;
				// Take first process in the buffer.
				struct process * p = removeFirst(&buffer[i].head, &buffer[i].tail);
				sem_post(&buffer[i].bufferSync);
				
				struct timeval pStartTime, pEndTime;
				// Run the job.
				runJob(p, &pStartTime, &pEndTime);
				// Time calculations.
				struct process * pProcessed = processJob(consumerID, p, pStartTime, pEndTime);
				
				// Check if the process has finished.
				if (pProcessed != NULL) {
					// Reschedule the process.
					sem_wait(&buffer[i].bufferSync);
					addLast(pProcessed, &buffer[i].head, &buffer[i].tail);
					buffer[i].currentBufferSize++;
					sem_post(&buffer[i].bufferSync);
				} else {
					// Update the number of consumed jobs.
					sem_wait(&sync);
					totalJobsConsumed++;
					sem_post(&sync);
					// Update the number of slots for jobs currently in the system.
					sem_post(&empty);
				}
				// Go back to the highest priority level.
				break;
				}
			sem_post(&buffer[i].bufferSync);
			
			// Check whether all jobs have been consumed.
			sem_wait(&sync);
			if (totalJobsConsumed == NUMBER_OF_JOBS) {
				c = 1;
				sem_post(&sync);
				break;
			}
			sem_post(&sync);
		}
		if (c) {
			break;
		}
	}
	pthread_exit(NULL);
}