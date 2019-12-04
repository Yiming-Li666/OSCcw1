#include "coursework.h"
#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// define the semaphores
sem_t buffer_locker;

struct buffer {
    struct element * head;
    struct element * tail;
    int counter;
};

struct buffer *pbuffer;
int isEnd = 0;
double responseTime;
double turnAroundTime;

void *producer(void *id) {
    int ID = *((int *)id);
    while(1) {
        //check if created job is less than MAX_NUMBER_OF_JOBS
        if (isEnd != 1) {
            sem_wait(&buffer_locker);
            //check if there is free space for new process
            if (pbuffer->counter != MAX_BUFFER_SIZE) {
                //create a new process, put it to the buffer
                struct process *temp = generateProcess();
                addLast(temp, &pbuffer->head, &pbuffer->tail);
                //one process added
                pbuffer->counter++;
                //print the result
                printf("Producer = %d, Items Produced = %d, New Process Id = %d, Burst Time = %d\n", ID, temp->iProcessId + 1, temp->iProcessId, temp->iInitialBurstTime);
                if (temp->iProcessId + 1 == MAX_NUMBER_OF_JOBS){
                    isEnd = 1;
                }
                //release the buffer
                sem_post(&buffer_locker);
                continue;
            }
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
    while (1) {
        struct timeval startTime;
        struct timeval endTime;
        struct process *temp = NULL;
        //to check if there is processes in the buffer, lock the buffer, otherwise keep waiting
        sem_wait(&buffer_locker);
        if (isEnd == 1 && pbuffer->counter == 0) {
            //quit immediately
            sem_post(&buffer_locker);
            break;
        }else {
            if (pbuffer->counter != 0) {
                //remove first process in the buffer
                temp = removeFirst(&pbuffer->head, &pbuffer->tail);
                pbuffer->counter--;
                //release the buffer
                sem_post(&buffer_locker);
            }else {
                sem_post(&buffer_locker);
            }
            //run the process
            if (temp != NULL) {
                runNonPreemptiveJob(temp, &startTime, &endTime);
                //print the result
                printf("Consumer = %d, Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %ld, Turn Around Time = %ld\n", ID, temp->iProcessId, temp->iPreviousBurstTime, temp->iRemainingBurstTime, getDifferenceInMilliSeconds(temp->oTimeCreated, startTime), getDifferenceInMilliSeconds(temp->oTimeCreated, endTime));
                responseTime += getDifferenceInMilliSeconds(temp->oTimeCreated, startTime);
                turnAroundTime += getDifferenceInMilliSeconds(temp->oTimeCreated, endTime);
                free(temp);
            }
        }
    }
    pthread_exit(NULL);
}

int main() {
    //init semaphore
    sem_init(&buffer_locker, 0, 1);
    
    //for producer thread
    pthread_t producers[NUMBER_OF_PRODUCERS];
    int pIDs[NUMBER_OF_PRODUCERS];
    pbuffer = (struct buffer*)malloc(sizeof(struct buffer)); 
    pbuffer->head = NULL;
    pbuffer->tail = NULL;
    pbuffer->counter = 0;
    int i;
    for (i = 0; i < NUMBER_OF_PRODUCERS; i++) {
        pIDs[i] = i;
    }
    for (int i = 0; i < NUMBER_OF_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer, (void *) &pIDs[i]);
    }
    
    //for consumer threads.
    pthread_t consumers[NUMBER_OF_CONSUMERS];
    int cIDs[NUMBER_OF_CONSUMERS];
    for (i = 0; i < NUMBER_OF_CONSUMERS; i++) {
        cIDs[i] = i;
    }
    for (i = 0; i < NUMBER_OF_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, (void *) &cIDs[i]);
    }
    
    // wait for all threads to finish.
    for (i = 0; i < NUMBER_OF_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (i = 0; i < NUMBER_OF_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    // Print average response time and average turn around time.
    printf("Average Response Time = %lf\n", responseTime / MAX_NUMBER_OF_JOBS);
    printf("Average Turn Around Time = %lf\n", turnAroundTime / MAX_NUMBER_OF_JOBS);
    // close the semaphore
    sem_close(&buffer_locker);
    return 0;
}