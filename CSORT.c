#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <stdbool.h>

#include "semun.h"
#include "shm_array.h"

static void set_semvalue(int sem_id);
static void del_semvalue(int sem_id);
static int semaphore_p(int sem_id);
static int semaphore_v(int sem_id);
void segmented_sort(char c_arr[3]);
bool sortedCheck(char *c_arr, int size);

int main() {
	/* Checks if user wants to be in debug mode */
	bool debug_flag;
	char input_one;
	printf("Would you like to be in debug mode (y/n)? ");
	scanf("%c", &input_one);
	if(input_one == 'y') {
		debug_flag = true;
	} else {
		debug_flag = false;
	}
	int c;
	while((c = getchar()) != '\n' && c != EOF) {}

	/* Initialize and Create shared memory */
	struct shared_use_st *shared_stuff;
	void *shared_memory = (void *)0;
	int shmid;
	shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
	if(shmid == -1) {
		perror("failed shmget");
		exit(EXIT_FAILURE);
	}
	shared_memory = shmat(shmid, (void *)0, 0);
	if(shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	shared_stuff = (struct shared_use_st *)shared_memory;

	/* Initialize the semaphores */
	shared_stuff->ar3_sem_id = semget((key_t)1235, 1, 0666 | IPC_CREAT);
	set_semvalue(shared_stuff->ar3_sem_id);
	shared_stuff->ar5_sem_id = semget((key_t)1236, 1, 0666 | IPC_CREAT);
	set_semvalue(shared_stuff->ar5_sem_id);
	
	/* Initialize sortedFlag */
	shared_stuff->sortedFlag = false;

	/* Receives user input for initial 7 characters */
	char input_two[8];
	printf("Please enter 7 characters as such (XbYUTwQ): ");
	fgets(input_two, 8, stdin);
	for(int i = 0; i < 7; i++) {
		input_two[i] = tolower(input_two[i]);
		shared_stuff->c_arr[i] = input_two[i];
	}
	printf("\n");

	/* Create processes A[1]-A[3], A[3]-A[5], A[5]-A[7] */
	pid_t pid1, pid2, pid3;

	/* Run processes pid1, pid2, and pid3 until the sortedFlag is true */
	while(!shared_stuff->sortedFlag) {
		/* Process pid1 deals with AR[1] to AR[3] */		
		pid1 = fork();
		switch(pid1) {
			case -1:
				perror("fork for pid1 failed");
				exit(EXIT_FAILURE);
			case 0:
				if(!shared_stuff->sortedFlag) {
					/* Comparison array for debugging */
					char beforeI[3] = {shared_stuff->c_arr[0], shared_stuff->c_arr[1], shared_stuff->c_arr[2]};
					/* Gain access and sort 3 elements of array */
					if(!semaphore_p(shared_stuff->ar3_sem_id)) {
						exit(EXIT_FAILURE);
					}
					char segment_one[3] = {shared_stuff->c_arr[0], shared_stuff->c_arr[1], shared_stuff->c_arr[2]};
					segmented_sort(segment_one);
					shared_stuff->c_arr[0] = segment_one[0];
					shared_stuff->c_arr[1] = segment_one[1];
					shared_stuff->c_arr[2] = segment_one[2];
					if(!semaphore_v(shared_stuff->ar3_sem_id)) { 
						exit(EXIT_FAILURE);
					}
					/* Debug option to print swapping info */
					if(debug_flag) {
						if((beforeI[0]!=segment_one[0])||(beforeI[1]!=segment_one[1])||(beforeI[2]!=segment_one[2])) {
							printf("P1: performed swapping\n");
						} else {
							printf("P1: no swapping\n");
						}
					}
				}
				exit(EXIT_SUCCESS);
		}
		/* Process pid2 deals with AR[3] to AR[5] */		
		pid2 = fork();
		switch(pid2) {
			case -1:
				perror("fork for pid2 failed");
				exit(EXIT_FAILURE);
			case 0:
				if(!shared_stuff->sortedFlag) {
					/* Comparison array for debugging */
					char beforeII[3] = {shared_stuff->c_arr[2], shared_stuff->c_arr[3], shared_stuff->c_arr[4]};
					/* Gain access and sort 3 elements of array */
					if(!semaphore_p(shared_stuff->ar3_sem_id)){ 		
						exit(EXIT_FAILURE);
					}
					if(!semaphore_p(shared_stuff->ar5_sem_id)) {
						exit(EXIT_FAILURE);
					}
					char segment_two[3] = {shared_stuff->c_arr[2], shared_stuff->c_arr[3], shared_stuff->c_arr[4]};
					segmented_sort(segment_two);
					shared_stuff->c_arr[2] = segment_two[0];
					shared_stuff->c_arr[3] = segment_two[1];
					shared_stuff->c_arr[4] = segment_two[2];
					
					if(!semaphore_v(shared_stuff->ar3_sem_id)) {
						exit(EXIT_FAILURE);
					}
					if(!semaphore_v(shared_stuff->ar5_sem_id)) {
						exit(EXIT_FAILURE);
					}
					/* Debug option to print swapping info */
					if(debug_flag) {
						if((beforeII[0]!=segment_two[0])||(beforeII[1]!=segment_two[1])||(beforeII[2]!=segment_two[2])) {
							printf("P2: performed swapping\n");
						} else {
							printf("P2: no swapping\n");
						}
					}
				}
				exit(EXIT_SUCCESS);
		}
		/* Process pid3 deals with AR[5] to AR[7] */		
		pid3 = fork();
		switch(pid3) {
			case -1:
				perror("fork for pid3 failed");
				exit(EXIT_FAILURE);
			case 0:
				if(!shared_stuff->sortedFlag) {
					/* Comparison array for debugging */
					char beforeIII[3] = {shared_stuff->c_arr[4], shared_stuff->c_arr[5], shared_stuff->c_arr[6]};
					/* Gain access and sort 3 elements of array */
					if(!semaphore_p(shared_stuff->ar5_sem_id)) {
						exit(EXIT_FAILURE);
					}
					char segment_three[3] = {shared_stuff->c_arr[4], shared_stuff->c_arr[5], shared_stuff->c_arr[6]};
					segmented_sort(segment_three);
					shared_stuff->c_arr[4] = segment_three[0];
					shared_stuff->c_arr[5] = segment_three[1];
					shared_stuff->c_arr[6] = segment_three[2];
					if(!semaphore_v(shared_stuff->ar5_sem_id)) {
						exit(EXIT_FAILURE);
					}
					/* Debug option to print swapping info */
					if(debug_flag) {
						if((beforeIII[0]!=segment_three[0])||(beforeIII[1]!=segment_three[1])||(beforeIII[2]!=segment_three[2])) {
							printf("P3: performed swapping\n");
						} else {
							printf("P3: no swapping\n");
						}
					}
				}
				exit(EXIT_SUCCESS);
		}

		/* Wait for all processes to complete */
		waitpid(pid1);
		waitpid(pid2);
		waitpid(pid3);

		/* Check if full array is sorted */
		char c_arr[7];
		for(int i = 0; i < 7; i++) {
			c_arr[i] = shared_stuff->c_arr[i];
		}
		shared_stuff->sortedFlag = sortedCheck(c_arr, 7);
	}
	
	/* Print sorted array */
	printf("\nThe sorted array is: %c, %c, %c, %c, %c, %c, %c\n", shared_stuff->c_arr[0], shared_stuff->c_arr[1], shared_stuff->c_arr[2], shared_stuff->c_arr[3], shared_stuff->c_arr[4], shared_stuff->c_arr[5], shared_stuff->c_arr[6]);

	/* Delete semaphores */
	del_semvalue(shared_stuff->ar3_sem_id);
	del_semvalue(shared_stuff->ar5_sem_id);

	/* Deallocate shared memory */
	if (shmdt(shared_memory) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	if (shmctl(shmid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

/* Initialize a semaphore given sem_id which is the id of the semaphore */
static void set_semvalue(int sem_id) {
	union semun sem_union;
	sem_union.val = 1;
	if(semctl(sem_id, 0, SETVAL, sem_union) == -1) fprintf(stderr, "failed to initialize sem");
}

/* Delete a semaphore given sem_id which is the id of the semaphore */
static void del_semvalue(int sem_id) {
	union semun sem_union;
	if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1) {
		fprintf(stderr, "Failed to delete semaphore\n");
	}
}

/* Acquire access to a semaphore given sem_id which is the id of the semaphore */
static int semaphore_p(int sem_id) {
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_p %d failed\n", sem_id);
		return(0);
	}
	return(1);
}

/* Releases access to a semaphore given sem_id which is the id of the semaphore */
static int semaphore_v(int sem_id) {
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_v failed\n");
		return(0);
	}
	return(1);
}

/* Sorts a segment of the array of size 3 into alphabetical order */
void segmented_sort(char c_arr[3]) {
	int i, j;	
	char temp;
	for(i = 0; i < 3; i++) {
		for(j = i + 1; j < 3; j++) {
			if(c_arr[i] > c_arr[j]) {
					temp = c_arr[i];
					c_arr[i] = c_arr[j];
					c_arr[j] = temp;
			}
		}
	}
}

/* Checks to see if given array and size of array is alphabetical sorted */
bool sortedCheck(char *c_arr, int size) {
	int i;
	for(i = 0; i < (size - 1); i++) {
		if(c_arr[i] > c_arr[i + 1]) {
			return false;
		}
	}
	return true;
}
