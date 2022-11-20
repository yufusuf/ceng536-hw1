#include "includes.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFSIZE 512
shmem_data_s * shmem;

int create_socket(int port_number);
void put_data_to_shmem(char *buf);


int port_number;

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        perror("error: input format ./logger shmmemaddr port");
        exit(1);
    }
    key_t shmemkey = SHMEM_KEY; // should be argv[1];
    port_number = atoi(argv[2]);
    
    // create socket with given port
    int sd = create_socket(port_number);

    // data receive buf and struct
    struct sockaddr_in recv_addr;
    int recv_len = 0;
    char buf[BUFSIZE]; memset(buf, 0, BUFSIZE);
    

    // connect to shared memory
    int shmid = shmget(SHMEM_KEY, 0, 0);
    if(shmid < 0)
    {
        perror("logger:shmid");
        fprintf(stderr, "logger on port %d\n", port_number);
        exit(1);
    }

    shmem = shmat(shmid, 0, 0);
    
    while(1)
    {
        // printf("logger waiting for data...\n");
        if((recv_len = recvfrom(sd, buf, BUFSIZE, 0, NULL ,NULL)) == -1)
        {
            perror("logger:rcv");
            exit(1);
        }
        printf("Data: %s", buf);
        put_data_to_shmem(buf);
    } 

    return 0;
}

int create_socket(int port_number)
{
    struct sockaddr_in sain; memset(&sain, 0, sizeof(sain));
    int sd;
    if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("logger:socket");
        exit(1);
    }

    sain.sin_family = AF_INET;
    sain.sin_port  = htons(port_number);
    sain.sin_addr.s_addr = htonl(INADDR_ANY); 

    if(bind(sd, (struct sockaddr *)&sain, sizeof(sain)) < 0) {
        perror("logger:bind");
        exit(1);
    }
    return sd;
}
void put_data_to_shmem(char *buf)
{
    // put anomality in
    // init n_notprocessed_analyzers
    // put content in
    // init mutex:w
    // signal analyzers
    
    pthread_mutex_lock(&(shmem->shmem_lock));
    if(shmem->n_analyzers > 0)
    {
        printf("logger with port %d got the shmem_lock\n", port_number);
        log_entry *l = &(shmem->que[shmem->buffer_index]);
        strncpy(l->content, buf, BUFSIZE);
        l->anomality = 0;
        l->timestamp = get_timestamp_ms();
        shmem->buffer_index++;
        shmem->item_count++;
        
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&(l->log_mutex), &attr);
    }
    pthread_mutex_unlock(&(shmem->shmem_lock));

    
    printf("logger with port %d released the shmem_lock\n", port_number);
    memset(buf, 0, BUFSIZE);
      

} 





