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

int main(int argc, char *argv[]) {
	// open the same shared memory
	int shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT, 0666); 
	if(shm_fd == -1)  {  
			printf("failed to open shared memory\n");  
			exit(1);
		 }  
	if(ftruncate(shm_fd, SIZE_OF_MEMORY) == -1) 
	{
		printf("failed to set size of memory\n");  
	}
	
	int * i_arr = mmap(NULL, SIZE_OF_MEMORY, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	// if ChildP1 has not processed for 10 times
	while(i_arr[21] <= 10){
		// if ChildP2 has not finished, do busy waiting
		if (i_arr[21]==i_arr[22]) {
			i_arr[i_arr[21] * 2 - 1] = i_arr[i_arr[21] * 2 - 2] * 2;
			// counter++
			i_arr[21] = i_arr[21] + 1;
		}
		
	};
	
}