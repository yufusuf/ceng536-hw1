#include "includes.h"

shmem_data_s *shmem;
void init_shmem(void);

int main()
{
    int shmid  = shmget(SHMEM_KEY, sizeof(shmem_data_s), IPC_CREAT | 0600);
    printf("shmid is %d\n", shmid); 
    
    if(shmid < 0)
    {
       fprintf(stderr, "shmget_error\n");
    }

    shmem = shmat(shmid, NULL, 0);
    
    init_shmem();

    while(1)
    {
        pthread_mutex_lock(&(shmem->shmem_lock));
        while(shmem->wake_reporter == 0)
        {
            pthread_cond_wait(&(shmem->reporter_cond), &(shmem->shmem_lock));
        }
        pthread_mutex_unlock(&(shmem->shmem_lock));
    }       
    return 0;
}
void init_shmem()
{
    memset(shmem, 0, sizeof(shmem_data_s));
    shmem->item_count = 0;
    shmem->n_analyzers = 0;
    shmem->buffer_index = 0;
    shmem->wake_reporter = 0;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(shmem->shmem_lock), &attr);

    pthread_mutex_init(&(shmem->analyzers_lock), &attr);
   

    pthread_condattr_t condattr;
    pthread_condattr_init(&condattr);
    pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&(shmem->analyzers_cond), &condattr);
    pthread_cond_init(&(shmem->reporter_cond), &condattr);
    return;
}
