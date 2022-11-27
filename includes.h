#ifndef __INCLUDES

#define __INCLUDES

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
#include <signal.h>
#define QUE_SIZE 100 
#define SHMEM_KEY 0x313131

typedef struct _log_entry{
    double anomality;
    int n_notprocessed_analyzers; 
    char content[512];
    pthread_mutex_t log_mutex;
} log_entry;

typedef struct _shmem_data{
    log_entry que[QUE_SIZE];
    int q_front;
    int q_rear;
    size_t item_count;
    int n_analyzers;
    pthread_mutex_t analyzers_lock;
    pthread_cond_t analyzers_cond;
    pthread_mutex_t shmem_lock;
    pthread_cond_t reporter_cond;
    pthread_cond_t que_full;
} shmem_data_s;
uint64_t get_timestamp_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((uint64_t)tv.tv_sec)*1000) + (tv.tv_usec/1000);
}
int isFull(shmem_data_s *s)
{
   return ((s->q_front == s->q_rear + 1) || 
           (s->q_front == 0 && s->q_rear == QUE_SIZE - 1));
}
#endif
