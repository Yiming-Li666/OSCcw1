#include "coursework.h"
#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Counting semaphore representing the number of slots for jobs currently in the system.
sem_t buffer_locker;

struct buffer {
    // Head and tail pointers of the buffer.
    struct element * head;
    struct element * tail;
    // Number of processes in the buffer.
    int count;
};
//the buffer
struct buffer buffer_for_all;
//check if the mission is over
int finish_flag = 0;

double allResponseTime, allTurnAroundTime;

void *producer(void *id) {
    int ID = *((int *)id);
    while(1) {
        //check if the mission has all been created
        if (finish_flag != 1) {
            //check if there is free space for new process
            sem_wait(&buffer_locker);
            if (buffer_for_all.count != MAX_BUFFER_SIZE) {
                //create a new process, put it to the buffer
                struct process *temp = generateProcess();
                addLast(temp, &buffer_for_all.head, &buffer_for_all.tail);
                //buffer_for_all.processes_produced++;
                buffer_for_all.count++;
                //print the result
                printf("Producer = %d, Items Produced = %d, New Process Id = %d, Burst Time = %d\n", ID, temp->iProcessId + 1, temp->iProcessId, temp->iInitialBurstTime);
                if (temp->iProcessId + 1 == MAX_NUMBER_OF_JOBS){
                    finish_flag = 1;
                }
                //end of the operation to the buffer, unlock
                sem_post(&buffer_locker);
                continue;
            }
            //end the operation to the buffer, unlock the buffer
            sem_post(&buffer_locker);
        }else {
            sem_post(&buffer_locker);
            break;
        }
    }
    pthread_exit(NULL);
}

void *consumer(void *id) {
    int ID = *((int *)id);
    //int consumer_finished = 0;
    while (1) {
        struct timeval processStartTime, processEndTime;
        struct process *temp = NULL;
        //to check if there is processes in the buffer, lock the buffer, otherwise keep waiting
        sem_wait(&buffer_locker);
        if (finish_flag == 1 && buffer_for_all.count == 0) {
            //quit immediately
            sem_post(&buffer_locker);
            break;
        }else {
            if (buffer_for_all.count != 0) {
                //take first process in the buffer, doing FCFS
                temp = removeFirst(&buffer_for_all.head, &buffer_for_all.tail);
                buffer_for_all.count--;
                //finish getting the process, free the space and unlock
                sem_post(&buffer_locker);
            }else {
                //finish the operation to the buffer, unlock the buffer
                sem_post(&buffer_locker);
            }
            //if gets the process, run it
            if (temp != NULL) {
                runNonPreemptiveJob(temp, &processStartTime, &processEndTime);
                //print the message
                printf("Consumer = %d, Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %ld, Turn Around Time = %ld\n", ID, temp->iProcessId, temp->iPreviousBurstTime, temp->iRemainingBurstTime, getDifferenceInMilliSeconds(temp->oTimeCreated, processStartTime), getDifferenceInMilliSeconds(temp->oTimeCreated, processEndTime));
                allResponseTime += getDifferenceInMilliSeconds(temp->oTimeCreated, processStartTime);
                allTurnAroundTime += getDifferenceInMilliSeconds(temp->oTimeCreated, processEndTime);
                free(temp);
            }
        }
    }
    pthread_exit(NULL);
}

int main() {
    printf("%d\n", MAX_BUFFER_SIZE);
    
    printf("%d\n", MAX_BUFFER_SIZE);
    //init semaphore
    sem_init(&buffer_locker, 0, 1);
    
    //for producer thread
    pthread_t producers_t[NUMBER_OF_PRODUCERS];
    int pIDs[NUMBER_OF_PRODUCERS];
    for (int i = 0; i < NUMBER_OF_PRODUCERS; i++) {
        pIDs[i] = i;
    }
    for (int i = 0; i < NUMBER_OF_PRODUCERS; i++) {
        pthread_create(&producers_t[i], NULL, producer, (void *) &pIDs[i]);
    }
    
    //for consumer threads.
    pthread_t consumers_t[NUMBER_OF_CONSUMERS];
    int cIDs[NUMBER_OF_CONSUMERS];
    for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
        cIDs[i] = i;
    }
    for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
        pthread_create(&consumers_t[i], NULL, consumer, (void *) &cIDs[i]);
    }
    
    // Tell the main thread to wait for all threads to finish.
    for (int i = 0; i < NUMBER_OF_PRODUCERS; i++) {
        pthread_join(producers_t[i], NULL);
    }
    for (int i = 0; i < NUMBER_OF_CONSUMERS; i++) {
        pthread_join(consumers_t[i], NULL);
    }
    
    // Print average response time and average turn around time.
    printf("Average Response Time = %lf\n", allResponseTime / MAX_NUMBER_OF_JOBS);
    printf("Average Turn Around Time = %lf\n", allTurnAroundTime / MAX_NUMBER_OF_JOBS);
    
    return 0;
}