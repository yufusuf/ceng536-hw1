#include "includes.h"
#include <regex.h>

shmem_data_s *shmem;
int last_analyzed_index = 0;
int main(int argc, char **argv)
{
    if(argc < 4)
    {
        perror("error: input format ./analyzer shmemadr regexp math");
        exit(1);
    }
    int shmid = shmget(SHMEM_KEY, 0, 0);
    if (shmid < 0)
    {
        perror("analyzer: shmget");
        return 1;
    }
    shmem = shmat(shmid, NULL, 0);
    
    regex_t regEx;
    if(regcomp(&regEx, argv[2], 0) != 0)
    {
        perror("regex compilation error");
        exit(1);
    }
    char * math_string = argv[3]; // '+float' or '*float'

    pthread_mutex_lock(&(shmem->shmem_lock));
    shmem->n_analyzers++;
    pthread_mutex_unlock(&(shmem->shmem_lock));
    
    while(1)
    {
        pthread_mutex_lock(&(shmem->analyzers_lock));
        while(shmem->item_count == 0)
        {
            printf("analyzer went to sleep\n");
            pthread_cond_wait(&(shmem->analyzers_cond), &(shmem->analyzers_lock));
        }
        printf("analyzer wake up\n");
        pthread_mutex_unlock(&(shmem->analyzers_lock));
        // look for unanalyzed log
        for(int i = last_analyzed_index; i < QUE_SIZE; i++)
        {
            log_entry *current = (shmem->que[i];j
    }
    return 0;

}
