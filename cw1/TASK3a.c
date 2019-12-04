#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "coursework.h"
#include "linkedlist.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>

struct buffer{
  struct element *head;
  struct element *tail;
  int counter;
};

struct buffer *pbuffer;
pthread_mutex_t buffer_locker = PTHREAD_MUTEX_INITIALIZER;  //locker
pthread_cond_t producerCond = PTHREAD_COND_INITIALIZER;  //Producer
pthread_cond_t consumerCond = PTHREAD_COND_INITIALIZER;  //Consumer

int isEnd = 0;
double respondTime;
double turnAroundTime;

void * Producer(void *ID){
    pthread_detach(pthread_self());
    long num = (long)ID;
    while(1){
        pthread_mutex_lock(&buffer_locker);
        //if created job is less than MAX_NUMBER_OF_JOBS
        if(isEnd != 1){
            // if process in the buffer is less than MAX_BUFFER_SIZE
            if(pbuffer->counter != MAX_BUFFER_SIZE){
                struct process *currentProcess = generateProcess();
                addLast(currentProcess,&pbuffer->head,&pbuffer->tail);
                // add one process
                pbuffer->counter++;
                printf("Producer = %d, Items Produced = %d, New Process Id = %d, Burst Time = %d\n",num,currentProcess->iProcessId+1,currentProcess->iProcessId,currentProcess->iInitialBurstTime);
                if(currentProcess->iProcessId + 1 == MAX_NUMBER_OF_JOBS){
                    // if created job is equal to MAX_NUMBER_OF_JOBS, set isEnd to 1
                    isEnd = 1;
                }
                // broadcast to the Consumer
                pthread_cond_broadcast(&consumerCond);
                pthread_mutex_unlock(&buffer_locker);
            }
            else{
                // if process in the buffer is euqal to MAX_BUFFER_SIZE
                pthread_cond_wait(&producerCond,&buffer_locker);
                pthread_mutex_unlock(&buffer_locker);
            }
        }
        else{
            pthread_mutex_unlock(&buffer_locker);
            break;
        }
    }
    pthread_exit(NULL);
}

void * Consumer(void *ID){
    pthread_detach(pthread_self());
    long num = (long)ID;
    while(1){
        struct process *currentProcess = NULL;
        struct timeval start;
        struct timeval end;
        pthread_mutex_lock(&buffer_locker);
        if(isEnd == 1 && pbuffer->counter == 0){
            //All job have been done
            pthread_mutex_unlock(&buffer_locker);
            break;
        }
        else{
            if(pbuffer->counter == 0){
                //no process in the buffer
                pthread_cond_wait(&consumerCond,&buffer_locker);
                pthread_mutex_unlock(&buffer_locker);
            }
            else{
                //do the process as FCFS
                currentProcess = removeFirst(&pbuffer->head,&pbuffer->tail);
                pthread_cond_broadcast(&producerCond);
                pthread_mutex_unlock(&buffer_locker);
            }
            if(currentProcess != NULL){
                //run the process and calculate time
                runNonPreemptiveJob(currentProcess,&start,&end);
                pbuffer->counter--;
                printf("Consumer = %d, Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %ld, Turn Around Time = %ld\n",num,currentProcess->iProcessId,currentProcess->iPreviousBurstTime,currentProcess->iRemainingBurstTime,getDifferenceInMilliSeconds(currentProcess->oTimeCreated,start),getDifferenceInMilliSeconds(currentProcess->oTimeCreated,end));
                respondTime += getDifferenceInMilliSeconds(currentProcess->oTimeCreated,start);
                turnAroundTime += getDifferenceInMilliSeconds(currentProcess->oTimeCreated,end);
                free(currentProcess);
            }
        }   
    }
    pthread_exit(NULL);
}

int main(int argc,char *argv[]) {

    pthread_t threadAll[NUMBER_OF_PRODUCERS+NUMBER_OF_CONSUMERS];
    int i;

    //initialize buffer
    pbuffer = (struct buffer*)malloc(sizeof(struct buffer)); 
    pbuffer->head = NULL;
    pbuffer->tail = NULL;
    pbuffer->counter = 0;

    // create producer threads
    for(i = 0; i < NUMBER_OF_PRODUCERS; i++){
        if(pthread_create(&threadAll[i],NULL,Producer,(void*)(long)i) == -1){
        printf("Creating Producer %d falied\n",i);
        exit(1);
        }
    }

    // create consumer threads
    for(i = 0;i < NUMBER_OF_CONSUMERS;i++){
        if(pthread_create(&threadAll[i+NUMBER_OF_PRODUCERS],NULL,Consumer,(void*)(long)i) == -1){
            printf("Creating cosumer %d failed\n",i);
            exit(1);
        }
    }
    // wait for all threads to finish.
    for(i = 0; i <NUMBER_OF_PRODUCERS+NUMBER_OF_CONSUMERS; i++){
        pthread_join(threadAll[i],NULL);
    }

    printf("Average respond time is: %lf\nsum of turn around time is: %lf\n",respondTime/MAX_NUMBER_OF_JOBS,turnAroundTime/MAX_NUMBER_OF_JOBS);
    return 0;
}
