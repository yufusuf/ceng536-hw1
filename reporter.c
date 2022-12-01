#include "includes.h"

shmem_data_s *shmem;
void init_shmem(void);
void print_que(void);
void sig_print(int signum)
{
    print_que();
}
int main(int argc,  char **argv)
{
    if(argc < 3)
    {
        perror("reporter input format ./reporter shmemaddr threshold");
        exit(1);
    }
    int threshold = atoi(argv[2]);
    int shmemaddr = atoi(argv[1]);
    // printf("%d\n", shmemaddr);
    int shmid  = shmget(shmemaddr, sizeof(shmem_data_s), IPC_CREAT | 0600);
    // printf("shmid is %d\n", shmid); 

    if(shmid < 0)
    {
        fprintf(stderr, "shmget_error\n");
    }

    shmem = shmat(shmid, NULL, 0);

    //signal(SIGINT, sig_print);
    init_shmem();
    while(1)
    {
        pthread_mutex_lock(&(shmem->shmem_lock));
        while(  shmem->q_front == -1 
                || shmem->que[shmem->q_front].n_notprocessed_analyzers != 0
                || shmem->working_analyzers > 0)
        {
            pthread_cond_wait(&(shmem->reporter_cond), &(shmem->shmem_lock));
        }
        int i = shmem->q_front;
        int  count = 0;
        int item_count = shmem->item_count;

        while(shmem->que[i].n_notprocessed_analyzers == 0 
                && count < item_count)
        {
            count++;
            // fprintf(stderr, "%d: anomality: %f\n", i, shmem->que[i].anomality);
            if(shmem->que[i].anomality >= threshold)
            {
                printf("%5.2f :%s\n", shmem->que[i].anomality, shmem->que[i].content);

            }
            // fprintf(stdout, "reporter deleting %d\n", i);
            memset(&(shmem->que[i]), 0, sizeof(log_entry));

            shmem->item_count--;
            if(shmem->q_front == shmem->q_rear)
            {
                shmem->q_front = -1;
                shmem->q_rear = -1;
            }
            else
                shmem->q_front = (shmem->q_front + 1) % QUE_SIZE;
            pthread_cond_signal(&(shmem->que_full));
            i = (i + 1) % QUE_SIZE; 
        } 
        pthread_mutex_unlock(&(shmem->shmem_lock));

    }       
    return 0;
}
void print_que(void)
{
    for(int i = 0; i < QUE_SIZE; i++)
    {
        log_entry * l = &(shmem->que[i]);
        fprintf(stderr, "%d: anom %f, n_npa %d, content %s\n", 
                i, l->anomality, l->n_notprocessed_analyzers, l->content); 
    }
}
void init_shmem()
{
    memset(shmem, 0, sizeof(shmem_data_s));
    shmem->item_count = 0;
    shmem->n_analyzers = 0;
    shmem->q_rear = -1;
    shmem->q_front = -1;
    shmem->working_analyzers = 0;
    shmem->logged_loggers = 0;
    shmem->n_loggers = 0;
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
    pthread_cond_init(&(shmem->que_full), &condattr);
    pthread_cond_init(&(shmem->wait_analyzers), &condattr);
    return;
}
