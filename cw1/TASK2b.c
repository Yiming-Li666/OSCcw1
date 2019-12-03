#include <stdlib.h>
#include <stdio.h>
#include "coursework.h"
#include "linkedlist.h"

int main(){
	struct element *pHead = NULL;
	struct element *pTail = NULL;
	struct element *pPointer = NULL;
    struct element *tempPointer = NULL;
    struct element *previousPointer = NULL;
	struct timeval oStartTime;
	struct timeval oEndTime; 
	double avgResponseTime = 0;
	double avgTurnAroundTime = 0;
	int responseTime = 0;
	int turnAroundTime = 0;
    int numberOfProcessLeft = NUMBER_OF_PROCESSES;
    int done = 0;
    
    // Display and all processes to the linked list
    printf("PROCESS LIST:\n");
    for(int i = 0; i < NUMBER_OF_PROCESSES; i++){
		struct process * oTemp = generateProcess();
		addLast(oTemp,&pHead,&pTail);
        printf("         Process Id = %d, Priority = %d, Initial Burst Time = %d, Remaining Burst Time = %d\n",((struct process*)(pTail->pData))->iProcessId,((struct process*)(pTail->pData))->iPriority,((struct process*)(pTail->pData))->iInitialBurstTime,((struct process*)(pTail->pData))->iRemainingBurstTime);
	}
    printf("END\n\n");

    // Reset the pointer to the first node
    pPointer = pHead;
 
    // Run the linked list once and calculate the response time
    for(int j = 0; j < NUMBER_OF_PROCESSES; j++){
        runPreemptiveJob((struct process*)pPointer->pData, &oStartTime, &oEndTime);	
        responseTime = getDifferenceInMilliSeconds(((struct process*)(pPointer->pData))->oTimeCreated,oStartTime);
        printf("Process Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Response Time = %d\n",((struct process*)(pPointer->pData))->iProcessId,((struct process*)(pPointer->pData))->iPriority,((struct process*)(pPointer->pData))->iPreviousBurstTime,((struct process*)(pPointer->pData))->iRemainingBurstTime,responseTime);
        avgResponseTime += responseTime;
        pPointer = pPointer->pNext;
    }

//  Repeat the linked list until it is empty
    while (pHead != NULL){
        
        done = 0;
        pPointer = pHead;

        // A linked list queue
        for(int k = 0; k < numberOfProcessLeft; k++){
       
            // Run the process
            runPreemptiveJob((struct process*)pPointer->pData, &oStartTime, &oEndTime);	
            turnAroundTime = getDifferenceInMilliSeconds(((struct process*)(pPointer->pData))->oTimeCreated,oEndTime);
            
            // If current node have not yet finish process but time slice is used up
            if(((struct process*)(pPointer->pData))->iRemainingBurstTime != 0){
                printf("Process Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d\n",((struct process*)(pPointer->pData))->iProcessId,((struct process*)(pPointer->pData))->iPriority,((struct process*)(pPointer->pData))->iPreviousBurstTime,((struct process*)(pPointer->pData))->iRemainingBurstTime);
            }
                        
            // If head node has completed the process
            if(((struct process*)pHead->pData)->iRemainingBurstTime == 0){       
                printf("Process Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turn Around Time = %d\n",((struct process*)(pPointer->pData))->iProcessId,((struct process*)(pPointer->pData))->iPriority,((struct process*)(pPointer->pData))->iPreviousBurstTime,((struct process*)(pPointer->pData))->iRemainingBurstTime,turnAroundTime);
                avgTurnAroundTime += turnAroundTime;
                free(pHead->pData);
                removeFirst(&pHead,&pTail);
                pPointer = pHead;
                done++;
                continue;
            }
            
            // If current node has completed process
            if(((struct process*)pPointer->pData)->iRemainingBurstTime == 0){ 
                printf("Process Id = %d, Priority = %d, Previous Burst Time = %d, Remaining Burst Time = %d, Turn Around Time = %d\n",((struct process*)(pPointer->pData))->iProcessId,((struct process*)(pPointer->pData))->iPriority,((struct process*)(pPointer->pData))->iPreviousBurstTime,((struct process*)(pPointer->pData))->iRemainingBurstTime,turnAroundTime);
                avgTurnAroundTime += turnAroundTime;
                // Remove the node that is completed
                tempPointer = pPointer;
                pPointer = previousPointer;
                pPointer -> pNext = previousPointer ->pNext ->pNext;
                free(tempPointer->pData);
                free(tempPointer);
                done++;
            }    

            // Move the node
            previousPointer = pPointer;
            pPointer = pPointer->pNext;
       }

       // Substract the completed node from the linked list
       if(done>0){
            numberOfProcessLeft = numberOfProcessLeft - done;
        }
    }

    // Calculate average response time and turnaround time
    printf("Average response time = %.6f\n",avgResponseTime/NUMBER_OF_PROCESSES);
    printf("Average turn around time = %.6f\n",avgTurnAroundTime/NUMBER_OF_PROCESSES);
}
