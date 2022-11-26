#include "includes.h"
#include <regex.h>

shmem_data_s *shmem;
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
    printf("shmid: %d\n", shmid);
    shmem = shmat(shmid, NULL, 0);
    
    regex_t regEx;
    if(regcomp(&regEx, argv[2], 0) != 0)
    {
        perror("regex compilation error");
        exit(1);
    }


    char * math_string = argv[3]; // '+float' or '*float'
    char op_type = math_string[0];
    float val = atof((math_string+1));

    int a_start;
    pthread_mutex_lock(&(shmem->shmem_lock));
    shmem->n_analyzers++;
    a_start  = (shmem->q_rear + 1) % QUE_SIZE;
    pthread_mutex_unlock(&(shmem->shmem_lock));
    
    while(1)
    {
        pthread_mutex_lock(&(shmem->analyzers_lock));
        while(shmem->wake_analyzers == 0)
        {
            printf("analyzer went to sleep\n");
            pthread_cond_wait(&(shmem->analyzers_cond), &(shmem->analyzers_lock));
        }
        printf("analyzer wake up\n");
        pthread_mutex_unlock(&(shmem->analyzers_lock));
        // TODO:
        // start iterating from head of the que
        // check if regexec() returns a match then do:
        // get the lock on the log_entry
        // do the math on anomality
        // if current log's n_not_processed_analyzers ==0
        // deque this item
        // singal to reporter check on this entry
        int i = a_start;
        while(i != ((shmem->q_rear + 1) % QUE_SIZE))
        {
            log_entry *l = &(shmem->que[i]);
            if(l == NULL)
            {
                fprintf(stderr, "entry is null\n");
                continue;
            }
            pthread_mutex_lock(&(l->log_mutex));
            if(regexec(&regEx, l->content, 0, NULL, 0) == 0)
            {
                fprintf(stderr, "--- analyzer on entry %d ---\n", i);
                fprintf(stderr, "analyzer matched %s\n", l->content);
                fprintf(stderr, "got anomaly %f\n", l->anomality);
                if(op_type == '+')
                    l->anomality += val;
                else if( op_type == '*')
                    l->anomality *= val;
                fprintf(stderr, "anomality updated to: %f\n", l->anomality);
                l->n_notprocessed_analyzers--;
                if(l->n_notprocessed_analyzers == 0)
                    pthread_cond_signal(&(shmem->reporter_cond)), 
                        printf("reporter signalled\n");
            }
            pthread_mutex_unlock(&(l->log_mutex)); 
            i = (i + 1) % QUE_SIZE;
        }
        pthread_mutex_lock(&(shmem->shmem_lock));
        shmem->wake_analyzers = 0;
        pthread_mutex_unlock(&(shmem->shmem_lock));

    }

    return 0;

}
