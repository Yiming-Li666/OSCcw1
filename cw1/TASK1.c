#include <stdio.h> 
#include <stdlib.h> 
#include <sys/mman.h> 
#include <sys/stat.h> 
#include <sys/shm.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/types.h>
#define SIZE_OF_MEMORY sizeof(int) 
#define SHARED_MEMORY_NAME "GDM123456"

int ChildP1(int n);
int ChildP2(int n);
int main() {
	int shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT, 0666); 
	if(shm_fd == -1){
		printf("failed to open shared memory\n");
		exit(1); 
	}
	if(ftruncate(shm_fd, SIZE_OF_MEMORY) == -1) {
		printf("failed to set size of memory\n");
	}
	int * i_ptr = mmap(NULL, SIZE_OF_MEMORY, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
	*i_ptr = ChildP1(MyRandom());
	printf("In round 1, RandInt = %d, child process 1,\n", *i_ptr);
	*i_ptr = ChildP2(*i_ptr);
	printf("In round 1, RandInt = %d, child process 2,\n", *i_ptr);

	
	if (munmap(i_ptr, SIZE_OF_MEMORY) == -1) perror("Error un-mmapping the file");{
		shm_unlink( SHARED_MEMORY_NAME );
		shmctl(shm_fd, IPC_RMID, 0);
	}
}

int MyRandom() {
	srand((int)time(0));
	int random = rand() % 15 + 1;
	return random;
}