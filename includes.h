#ifndef __INCLUDES

#define __INCLUDES

#define SHMEM_KEY 0x313131
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#define QUE_SIZE 10000

typedef struct _log_entry{
    double anomality;
    size_t n_notprocessed_analyzers; 
    char content[512];
    uint64_t timestamp;
    pthread_mutex_t log_mutex;
} log_entry;

typedef struct _shmem_data{
    log_entry que[QUE_SIZE];
    size_t buffer_index;
    size_t item_count;
    int n_analyzers;
    char wake_reporter;
    pthread_mutex_t analyzers_lock;
    pthread_cond_t analyzers_cond;
    pthread_mutex_t shmem_lock;
    pthread_cond_t reporter_cond;
} shmem_data_s;

uint64_t get_timestamp_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((uint64_t)tv.tv_sec)*1000) + (tv.tv_usec/1000);
}


#endif
