#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "includes.h"



int main()
{
    int shmid  = shmget(SHMEM_KEY, 100*sizeof(char), IPC_CREAT | 0600);
    
    if(shmid < 0)
    {
       fprintf(stderr, "shmget_error\n");
    }
    char * shared_mem_pointer = shmat(shmid, NULL, 0);
    shared_mem_pointer[0] = 'h';
    shared_mem_pointer[1] = 'e';
    shared_mem_pointer[2] = 'l';
    shared_mem_pointer[3] = 'l';
    shared_mem_pointer[4] = 'o';
    shared_mem_pointer[5] = 0;

        
    return 0;
}

