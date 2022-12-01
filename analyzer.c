#include "includes.h"
#include <regex.h>

shmem_data_s *shmem;
char is_terminated = 0;
void sig_handler(int signum)
{
    printf("analyzer in sigint handler\n");
    is_terminated = 1;
}
int main(int argc, char **argv)
{
    if(argc < 4)
    {
        perror("error: input format ./analyzer shmemadr regexp math");
        exit(1);
    }
    int shmid = shmget(atoi(argv[1]), 0, 0);
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
    signal(SIGINT, sig_handler);
    char signal_rprt = 0;
    while(!is_terminated)
    {
        pthread_mutex_lock(&(shmem->analyzers_lock));
        printf("analyzer sleeping\n");
        pthread_cond_wait(&(shmem->analyzers_cond), &(shmem->analyzers_lock));
        pthread_mutex_unlock(&(shmem->analyzers_lock));

        int i = a_start;
        while(i != ((shmem->q_rear + 1) % QUE_SIZE))
        {
            fprintf(stderr, "analyzing %d\n", i);
            log_entry *l = &(shmem->que[i]);
            if(!l->isFull)
            {
                fprintf(stderr, "entry is null\n");
                i = (i + 1) % QUE_SIZE;
                continue;
            }
            pthread_mutex_lock(&(l->log_mutex));
            l->n_notprocessed_analyzers--;
            if(l->n_notprocessed_analyzers == 0)
            {
                printf("reporter signalled\n");
                pthread_cond_signal(&(shmem->reporter_cond));
            }
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
            }
            pthread_mutex_unlock(&(l->log_mutex)); 
            i = (i + 1) % QUE_SIZE;
        }
        //if(signal_rprt)
        //{
        //    pthread_cond_signal(&(shmem->reporter_cond)); 
        //    signal_rprt = 0 ;
        //}
        //pthread_mutex_lock(&(shmem->shmem_lock));
        //if( --shmem->working_analyzers == 0)
        //{
        //    shmem->logged_loggers = 0;
        //    pthread_cond_broadcast(&(shmem->wait_analyzers));
        //    pthread_cond_signal(&(shmem->reporter_cond));
        //}
        //pthread_mutex_unlock(&(shmem->shmem_lock));
        pthread_mutex_lock(&(shmem->shmem_lock));
        if( --shmem->working_analyzers == 0)
        {
            pthread_cond_signal(&(shmem->reporter_cond));
            shmem->logged_loggers = 0;
            pthread_cond_broadcast(&(shmem->wait_analyzers));
            printf("reporter signalled\n");

        }
        pthread_mutex_unlock(&(shmem->shmem_lock));

    }
    pthread_mutex_lock(&(shmem->shmem_lock));
    shmem->n_analyzers--;
    pthread_mutex_unlock(&(shmem->shmem_lock));
    return 0;

}
