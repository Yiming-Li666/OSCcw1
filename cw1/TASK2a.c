#include <stdlib.h>
#include <stdio.h>
#include "coursework.h"
#include "linkedlist.h"

int main(){
	
	struct element *pHead = NULL;
	struct element *pTail = NULL;
	struct element *pPointer = NULL;
	struct timeval oStartTime;
	struct timeval oEndTime; 
	double totalResponseTime = 0;
	double totalTurnAroundTime = 0;
	double averageResponseTime = 0;
	double averageTurnAroundTime = 0;
	int responseTime = 0;
	int turnAroundTime = 0;
	struct process * oTemp;


	// Add all the processes to the linked list
	for(int i = 0; i < NUMBER_OF_PROCESSES; i++){
		oTemp = generateProcess();
		addLast(oTemp,&pHead,&pTail);
	}	
	
	// Set the pointer to the first node
	pPointer = pHead;

	for(int j = 0; j < NUMBER_OF_PROCESSES; j++){
		// Run the linked list
		runNonPreemptiveJob((struct process*)pPointer->pData, &oStartTime, &oEndTime);	
		//calculate the response time and turnaround time
		responseTime = getDifferenceInMilliSeconds(((struct process*)(pPointer->pData))->oTimeCreated,oStartTime);
		turnAroundTime = getDifferenceInMilliSeconds(((struct process*)(pPointer->pData))->oTimeCreated,oEndTime);
		totalResponseTime = totalResponseTime + responseTime;
		totalTurnAroundTime = totalTurnAroundTime + turnAroundTime; 
		printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %d, Turn Around Time = %d\n",((struct process*)(pPointer->pData))->iProcessId,((struct process*)(pPointer->pData))->iInitialBurstTime,((struct process*)(pPointer->pData))->iRemainingBurstTime,responseTime,turnAroundTime);
		pPointer = pPointer->pNext; 
	}

	// Calculate the average response and turnaround time, keep 6 dicimal places
	averageResponseTime = totalResponseTime/NUMBER_OF_PROCESSES;
	averageTurnAroundTime = totalTurnAroundTime/NUMBER_OF_PROCESSES;
	printf("Average response time = %.6f\n",averageResponseTime);
	printf("Average turn around time = %.6f\n",averageTurnAroundTime);

	// Set the pointer to the first node
	pPointer = pHead;

	// Free and release all the node from the linked list
	for(int k = 0; k < NUMBER_OF_PROCESSES; k++){
		free(pPointer->pData);
		pPointer = pPointer->pNext;
		removeFirst(&pHead,&pTail);
	}
}
