#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "coursework.h"
#include "linkedlist.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>


//buffer struct
struct buffer{
    struct element *pHead;
    struct element *pTail;
    int counterBufferJob;
};

struct priorityQueue
{
    int priorityId;
    struct element *PQpHead;
    struct element *PQpTail;
    int counter;
};

struct buffer *pbuffer;
pthread_mutex_t buffer_locker = PTHREAD_MUTEX_INITIALIZER;  //locker
pthread_cond_t producerCond = PTHREAD_COND_INITIALIZER;  //Producer
pthread_cond_t consumerCond = PTHREAD_COND_INITIALIZER;  //Consumer

int isEnd = 0;
double sumOfRespondTime = 0;
double sumOfTurnAroundTime = 0;


void *Producer(void *ID){
    pthread_detach(pthread_self());
    long num = (long)ID;
    while(1){
        pthread_mutex_lock(&buffer_locker);
        if(isEnd != 1){
            //created job if less than MAX_NUMBER_OF_JOBS
            if(pbuffer->counterBufferJob != MAX_BUFFER_SIZE){
                //element of single list less than MAX_BUFFER_SIZE
                struct process *currentProcess = generateProcess();

                //find the level of the PQ
                struct element *currentPriority = pbuffer->pHead;
                int j = 0;
                for(j = 0; j < currentProcess->iPriority;j++){
                    currentPriority = currentPriority->pNext;
                }

                //adding to last of the process single linked list 
                struct priorityQueue *currentPQ = currentPriority->pData;

                addLast(currentProcess,&(currentPQ->PQpHead),&(currentPQ->PQpTail));
                currentPQ->counter += 1;
                pbuffer->counterBufferJob++;
                printf("Producer = %d, Items Produced = %d, New Process Id = %d, Burst Time = %d, Priority = %d\n",num,currentProcess->iProcessId+1,currentProcess->iProcessId,currentProcess->iInitialBurstTime,currentProcess->iPriority);
                if(currentProcess->iProcessId + 1 == MAX_NUMBER_OF_JOBS){
                    //Last Job have been created
                    isEnd = 1;
                }
                pthread_cond_broadcast(&consumerCond);
                pthread_mutex_unlock(&buffer_locker);
            }
            else{
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



void *Consumer(void *ID){
    pthread_detach(pthread_self());
    long num = (long)ID;
    while(1){
        struct process *currentProcess = NULL;
        struct priorityQueue *currentPQ = NULL;
        struct timeval start,end;
        pthread_mutex_lock(&buffer_locker);

        if(isEnd == 1 && pbuffer->counterBufferJob == 0){
            //All created job have been run
            pthread_mutex_unlock(&buffer_locker);
            break;
        }
        else{
            if(pbuffer->counterBufferJob == 0){
                //no process in the buffer
                pthread_cond_wait(&consumerCond,&buffer_locker);
                pthread_mutex_unlock(&buffer_locker);
            }
            else{
                //have processes in the buffer
                struct element *currentPriority = pbuffer->pHead;
                while(currentPriority != NULL){
                    currentPQ = currentPriority->pData;
                    if(currentPQ->counter == 0){    //no element had been added into this priority
                        currentPriority = currentPriority->pNext;
                    }
                    else{    
                        currentProcess = removeFirst(&(currentPQ->PQpHead),&(currentPQ->PQpTail));
                        currentPQ->counter -= 1;
                        break;
                    }
                }
                pthread_cond_broadcast(&producerCond);
                pthread_mutex_unlock(&buffer_locker);


                if(currentProcess != NULL){
                    //do something with currentProcess
                    struct process *executedProcess = currentProcess; 
                    runPreemptiveJob(executedProcess,&start,&end);
                    //The prcess is the first time to execute and the execute time is larger than TIME_SLICE
                    if(executedProcess->iPreviousBurstTime == executedProcess->iInitialBurstTime && executedProcess->iInitialBurstTime > TIME_SLICE){
                        printf("Consumer = %d, Process Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Respond Time = %d\n",num,executedProcess->iProcessId,currentPQ->priorityId,executedProcess->iPreviousBurstTime,executedProcess->iRemainingBurstTime,getDifferenceInMilliSeconds(currentProcess->oTimeCreated,start));
                        sumOfRespondTime += getDifferenceInMilliSeconds(currentProcess->oTimeCreated,start);

                    }
                    else if(executedProcess->iPreviousBurstTime == executedProcess->iInitialBurstTime && TIME_SLICE >= executedProcess->iInitialBurstTime > 0){
                        //The process is the first time to execute and the execute time is larger than 0 and less than TIME_SLICE 
                        printf("Consumer = %d, Process Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Respond Time = %d, TurnAroundTime = %d\n",num,executedProcess->iProcessId,currentPQ->priorityId,executedProcess->iPreviousBurstTime,executedProcess->iRemainingBurstTime,getDifferenceInMilliSeconds(currentProcess->oTimeCreated,start),getDifferenceInMilliSeconds(currentProcess->oTimeCreated,end));
                        sumOfRespondTime += getDifferenceInMilliSeconds(currentProcess->oTimeCreated,start);
                        sumOfTurnAroundTime += getDifferenceInMilliSeconds(currentProcess->oTimeCreated,end);

                    }
                    else if(executedProcess->iRemainingBurstTime == 0){
                        //The process is running end
                        printf("Consumer = %d, Process Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, TurnAroundTime = %d\n",num,executedProcess->iProcessId,currentPQ->priorityId,executedProcess->iPreviousBurstTime,executedProcess->iRemainingBurstTime,getDifferenceInMilliSeconds(currentProcess->oTimeCreated,end));
                        sumOfTurnAroundTime += getDifferenceInMilliSeconds(currentProcess->oTimeCreated,end);
                    }
                    else{
                        printf("Consumer = %d, Process Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n",num,executedProcess->iProcessId,currentPQ->priorityId,executedProcess->iPreviousBurstTime,executedProcess->iRemainingBurstTime);
                    }
    

                    //If the remaining time to execute is bigger than 0,add this process to the end of single list
                    if(currentProcess->iRemainingBurstTime > 0){
                        addLast(currentProcess,&(currentPQ->PQpHead),&(currentPQ->PQpTail));
                        currentPQ->counter += 1;
                    }
                    else{
                        pbuffer->counterBufferJob--;
                        free(currentProcess); 
                    }
                }
            } 
        }
    }
    pthread_exit(NULL);
}

int main(int argc,char *argv[]){
    srand((unsigned)time(NULL));
    pthread_t theardAll[NUMBER_OF_PRODUCERS+NUMBER_OF_CONSUMERS];
    int i;

    //buffer initialize
    pbuffer = (struct buffer*)malloc(sizeof(struct buffer)); 
    pbuffer->pHead = NULL;
    pbuffer->pTail = NULL;
    pbuffer->counterBufferJob = 0;

    //creating priority-level single linked list
    for(i=0; i < MAX_PRIORITY;i++){
        struct priorityQueue *currentPQ = (struct priorityQueue*) malloc(sizeof(struct priorityQueue));

        //initialize the element in the currentPQ
        currentPQ->priorityId = i;
        currentPQ->PQpHead = NULL;
        currentPQ->PQpTail = NULL;
        currentPQ->counter = 0;                                                                                          

        //add this currentPQ to the end of Priority Queue single linked list
        addLast(currentPQ,&pbuffer->pHead,&pbuffer->pTail);
    }

    //create producer threads
    for(i = 0; i < NUMBER_OF_PRODUCERS; i++){
        if(pthread_create(&theardAll[i],NULL,Producer,(void*)(long)i) == -1){
            printf("Creating Producer %d falied\n",i);
            exit(1);
        }
    }

    //create consumer  threads
    for(i = 0;i < NUMBER_OF_CONSUMERS;i++){
        if(pthread_create(&theardAll[i+NUMBER_OF_PRODUCERS],NULL,Consumer,(void*)(long)i) == -1){
            printf("Creating cosumer %d failed\n",i);
            exit(1);
        }
    }

    //wait until all the process run out
    for(i = 0; i <NUMBER_OF_PRODUCERS+NUMBER_OF_CONSUMERS; i++){
        pthread_join(theardAll[i],NULL);
    }

    printf("Average respond time is: %lf\nAverage of turn around time is: %lf\n",sumOfRespondTime/MAX_NUMBER_OF_JOBS,sumOfTurnAroundTime/MAX_NUMBER_OF_JOBS);
    return 0;
}

