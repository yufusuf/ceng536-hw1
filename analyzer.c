#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include "includes.h"
#include <string.h>

int main()
{
    int shmid = shmget(SHMEM_KEY, 0, 0);
    if (shmid < 0)
    {
        perror("shmget");
        return 1;
    }
    char *hello_s = malloc(6*sizeof(char));
    strncpy(hello_s, shmat(shmid, NULL, 0), 6);
    fprintf(stdout, "%s\n", hello_s);
    return 0;

}
