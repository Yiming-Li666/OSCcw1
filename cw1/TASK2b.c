#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "coursework.h"
#include "linkedlist.h"
#include <time.h>

struct priorityQueue {
  int priorityId;
  struct element *pHead;
  struct element *pTail;
  int counter;
};

int main() {
    // for Prioirity Queue
    struct element *PQHead = NULL;
    struct element *PQTail = NULL;
    // for process linked list
    struct element *pHead = NULL;
    struct element *pTail = NULL;
    struct element *processPointer = NULL;
    double averageResponseTime = 0;
    double averageTurnAroundTime = 0;
    double responseTime = 0;
    double turnAroundTime = 0;

    //create a single linked list to store all the Priority Queue
    int i;
    for(i=0; i < MAX_PRIORITY;i++){
        struct priorityQueue *currentPQ = (struct priorityQueue*) malloc(sizeof(struct priorityQueue));
        currentPQ->priorityId = i;
        currentPQ->pHead = NULL;
        currentPQ->pTail = NULL;
        currentPQ->counter = 0;                                                                                          
        //add this currentPQ to the endTime of Priority Queue
        addLast(currentPQ,&PQHead,&PQTail);
    }

    // Add all the processes to linked list
    for(i = 0; i <NUMBER_OF_PROCESSES;i++){ 
        struct process * newProcess = generateProcess();
        addLast(newProcess,&pHead,&pTail);
        // Set the pointer to the first priority
        processPointer = PQHead;
        // looking for the corresponding priority
        int j = 0;
        for(j = 0; j < newProcess->iPriority;j++){
            processPointer = processPointer->pNext;
        }
        //Add all the processes to the priority queue
        struct priorityQueue *currentPQ = processPointer->pData;
        addLast(newProcess,&(currentPQ->pHead),&(currentPQ->pTail));
        currentPQ->counter = currentPQ->counter + 1;
    }

    //-------------------------------------------------------------------
    //print out the priority and process
    printf("PROCESS LIST:\n");
    // Set the pointer to the first priority
    processPointer = PQHead;
    while(processPointer != NULL){
        struct priorityQueue *currentPQ = processPointer->pData;
        if(currentPQ->counter != 0){
            printf("Priority %d:\n",currentPQ->priorityId);
            struct element *currentProcessList = currentPQ->pHead;
            while(currentProcessList != NULL){
                struct process *currentProcess = currentProcessList->pData;
                printf("\tProcess Id = %d, Priority = %d, Initial Burst Time = %d, Remaining Burst Time = %d\n",currentProcess->iProcessId,currentPQ->priorityId,currentProcess->iInitialBurstTime,currentProcess->iRemainingBurstTime);
                currentProcessList = currentProcessList->pNext;
            }
        }
        processPointer = processPointer->pNext;
    }
    printf("endTime\n\n");

    //-------------------------------------------------------------------
    //run process
    processPointer = PQHead;
    while(processPointer != NULL){
        struct priorityQueue *currentPQ = processPointer->pData;
        //if no element inside this priority
        if(currentPQ->counter == 0){    
            processPointer = processPointer->pNext;
            continue;
        }
        else{
            struct process *currentProcess = currentPQ->pHead->pData;
            struct timeval startTime,endTime;
            runPreemptiveJob(currentProcess,&startTime,&endTime);
            struct process *executedProcess = removeFirst(&(currentPQ->pHead),&(currentPQ->pTail));
      
            //The prcess is the first time to execute and the execute time is larger than TIME_SLICE
            if(executedProcess->iPreviousBurstTime == executedProcess->iInitialBurstTime && executedProcess->iInitialBurstTime > TIME_SLICE){
                printf("\tProcess Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Respond Time = %d\n",executedProcess->iProcessId,currentPQ->priorityId,executedProcess->iPreviousBurstTime,executedProcess->iRemainingBurstTime,getDifferenceInMilliSeconds(currentProcess->oTimeCreated,startTime));
                responseTime = responseTime + getDifferenceInMilliSeconds(currentProcess->oTimeCreated,startTime);
            }
            //The process is the first time to execute and the execute time is larger than 0 and less than TIME_SLICE 
            else if(executedProcess->iPreviousBurstTime == executedProcess->iInitialBurstTime && TIME_SLICE >= executedProcess->iInitialBurstTime > 0){  
                printf("\tProcess Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Respond Time = %d, TurnAroundTime = %d\n",executedProcess->iProcessId,currentPQ->priorityId,executedProcess->iPreviousBurstTime,executedProcess->iRemainingBurstTime,getDifferenceInMilliSeconds(currentProcess->oTimeCreated,startTime),getDifferenceInMilliSeconds(currentProcess->oTimeCreated,endTime));
                responseTime = responseTime + getDifferenceInMilliSeconds(currentProcess->oTimeCreated,startTime);
                turnAroundTime = turnAroundTime + getDifferenceInMilliSeconds(currentProcess->oTimeCreated,endTime);
            }
            //The process is running endTime
            else if(executedProcess->iRemainingBurstTime == 0){
                printf("\tProcess Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, TurnAroundTime = %d\n",executedProcess->iProcessId,currentPQ->priorityId,executedProcess->iPreviousBurstTime,executedProcess->iRemainingBurstTime,getDifferenceInMilliSeconds(currentProcess->oTimeCreated,endTime));
                turnAroundTime = turnAroundTime + getDifferenceInMilliSeconds(currentProcess->oTimeCreated,endTime);
            }
            else{
                printf("\tProcess Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n",executedProcess->iProcessId,currentPQ->priorityId,executedProcess->iPreviousBurstTime,executedProcess->iRemainingBurstTime);
            }
           
            //If the remaining time to execute is bigger than 0,add this process to the endTime of single list
            if(executedProcess->iRemainingBurstTime > 0){
                addLast(executedProcess,&(currentPQ->pHead),&(currentPQ->pTail));
            }
            else{
                free(executedProcess);
                currentPQ->counter--;
            }
        }
    }
    //Calculating the average respondtime and turnAroundTime
    averageResponseTime = responseTime/NUMBER_OF_PROCESSES;
    averageTurnAroundTime = turnAroundTime/NUMBER_OF_PROCESSES;
    printf("Average of respond time is: %.6f\nAverage of turn around time is: %.6f\n",averageResponseTime,averageTurnAroundTime);

    return 0;
}
