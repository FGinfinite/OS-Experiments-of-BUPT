#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MYKEY 1145
#define MAXQUEUE 10
typedef struct
{
    long msgtyp;
    size_t mgssz;
    void *msg;
} msg_data;

typedef struct
{
    msg_data data;
    msg_node *next;
} msg_node;

typedef struct
{
    key_t key;
    msg_node *head;
} msg_queue;

msg_queue queues[MAXQUEUE];

int main()
{
    int queue_count = 0;

    int shmid = shmget(MYKEY, sizeof(bool) + sizeof(key_t), IPC_CREAT | 0666);
    void *shm = shmat(shmid, NULL, 0);
    bool *newQueue = (bool *)shm;
    *newQueue = false;
    key_t *newKey = (key_t *)(shm + sizeof(bool));


    while (1)
    {
        while (*newQueue != true)
        {
        }
        queues[queue_count].key =*newKey;
        queues[queue_count].head=NULL;


    }
}