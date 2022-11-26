#include "includes.h"

shmem_data_s *shmem;
void init_shmem(void);
void print_que(void);
int main(int argc,  char **argv)
{
    if(argc < 3)
    {
        perror("reporter input format ./reporter shmemaddr threshold");
        exit(1);
    }
    int threshold = atoi(argv[2]);
    int shmemaddr = atoi(argv[1]);
    // TODO: shmemaddr instead of shmem_key
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
        while(  shmem->q_front == -1 
                || shmem->que[shmem->q_front].n_notprocessed_analyzers != 0)
        {
            fprintf(stderr, "reporter sleeping %d\n", shmem->q_front);
            pthread_cond_wait(&(shmem->reporter_cond), &(shmem->shmem_lock));
        }
        fprintf(stderr, "report has wakenup\n");
        pthread_mutex_unlock(&(shmem->shmem_lock));
        // start looping from que's head till end
        // if an entry found with n_not_processed_analyzers == 0
        // report it if anomality>threshold
        // else continue
        int i = shmem->q_front;
        int start = i;
        int item_count = shmem->item_count;
        while(shmem->que[i].n_notprocessed_analyzers == 0 
                && (i - start + 1) != item_count + 1)
        {
            // fprintf(stderr, "%d: anomality: %f\n", i, shmem->que[i].anomality);
            if(shmem->que[i].anomality >= threshold)
            {
                printf("%5.2f :%s\n", shmem->que[i].anomality, shmem->que[i].content);
            }
            memset(&(shmem->que[i]), 0, sizeof(log_entry));
            shmem->item_count--;
            if(shmem->q_front == shmem->q_rear)
            {
                shmem->q_front = -1;
                shmem->q_rear = -1;
            }
            else
                shmem->q_front = (shmem->q_front + 1) % QUE_SIZE;
            i = (i + 1) % QUE_SIZE; 
        } 


    }       
    return 0;
}
void print_que(void)
{
    for(int i = shmem->q_front ; i <= shmem->q_rear; (i = (i + 1) % QUE_SIZE))
    {
        log_entry * l = &(shmem->que[i]);
        fprintf(stderr, "%d: anom %f, n_npa %lu, content %s\n", 
                i, l->anomality, l->n_notprocessed_analyzers, l->content); 
    }
}
void init_shmem()
{
    memset(shmem, 0, sizeof(shmem_data_s));
    shmem->item_count = 0;
    shmem->n_analyzers = 0;
    shmem->wake_reporter = 0;
    shmem->q_rear = -1;
    shmem->q_front = -1;
    shmem->wake_analyzers = 0;
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
