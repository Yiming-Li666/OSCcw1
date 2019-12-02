#include <stdio.h> 
#include <stdlib.h> 
#include <sys/mman.h> 
#include <sys/stat.h> 
#include <sys/shm.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <string.h>
#define SIZE_OF_MEMORY sizeof(int) 
#define SHARED_MEMORY_NAME "GDM123456"

// int ChildP1(int n);
// int ChildP2(int n);
int main() {
	int round = 10;
	int process_num = 2;
	int shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT, 0666); 
	pid_t pid = 0;
	int status, wpid[2];
	// open shared memory
	if(shm_fd == -1){
		printf("failed to open shared memory\n");
		exit(1); 
	}
	if(ftruncate(shm_fd, SIZE_OF_MEMORY) == -1) {
		printf("failed to set size of memory\n");
	}
	int * i_arr = mmap(NULL, SIZE_OF_MEMORY, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
	// i_arr[0] is the original random value
	// i_arr[21] is the counter of childP1
	// int t;
	// for(t = 0; t<20; t++){
	// 	i_arr[t] = -1;
	// }
	i_arr[21] = 1;
	// i_arr[22] is the counter of childP2
	i_arr[22] = 1;
	// ********************************
	int i;
	for(i = 0; i < process_num; i++){
		pid_t pid = fork();
		wpid[i] = pid;
		if(pid == -1) {
			printf("fork () error\n");
			exit(1);
		} 
		else if(pid == 0) {
			//for 10 times
			int j;
			//for( j = 1; j < round + 1; j++){
				//if(i == 0){
			if(i == 0){
					// Child process 1
					// char *envp[] = {* i_arr,j,child1_flag,child2_flag,NULL};
    	// 			char *argv_send[] = {"./ChildP1",NULL};
					// evecve("./ChildP1",argv_send,envp);
					execl("./ChildP1","ChildP1",NULL);
					printf("P1 %d\n", j);
					// i_arr[j] = (i_arr[j-1]) * 2;
					// child1_flag = 0;
					// child2_flag = 1;
			}
				//else if(i == 1){
			else if(i == 1){
					// Child process 2
					// int *envp[] = {* i_arr,j,child1_flag,child2_flag,NULL};
    	// 			char *argv_send[] = {"./ChildP2",NULL};
					// evecve("./ChildP1",argv_send,envp);
					execl("./ChildP2","ChildP2",NULL);
					printf("P2 %d\n", j);
					// i_arr[j] = (i_arr[j-1]) - 10;
					// child1_flag = 1;
					// child2_flag = 0;
			}
			//}
            exit(1);
		} 

		else if(pid > 0){
			// get random value
			i_arr[0] = MyRandom();
		}
	}
	// ********************************
	// parent wait for all the child to finish
	for (i = 0; i < process_num; i++){
		waitpid(wpid[i], &status, WUNTRACED);
	}
	printf("The RandInt = %d,created by the parent process\n",i_arr[0]);
	for (i = 0; i < (round * 2); i++)
	{
		printf("In round %d, RandInt = %d, child process %d\n", i/2+1, i_arr[i+1], i%2 + 1);
		//printf("%d,%d,%d\n",i/2+1,i_arr[i], (i+1)%2 + 1);
		//printf("In round %d, RandInt = %d, child process %d,\n",i, *(i_arr[0]+i), (i+1) % 2 + 1);
	}
	// close shared memory
	if (munmap(i_arr, SIZE_OF_MEMORY) == -1)
		perror("Error un-mmapping the file");
	shm_unlink( SHARED_MEMORY_NAME );
	shmctl(shm_fd, IPC_RMID, 0);
	
}

int MyRandom() {
	srand((int)time(0));
	int random = rand() % 20 + 1;
	return random;
}