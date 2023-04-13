#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "pool.h"

pthread_mutex_t mutex;

typedef struct
{
    int i;
    pthread_t tid;
} node;

void *setmem(void *a)
{
    pthread_mutex_lock(&mutex);
    static int count = 0;
    ((node *)a)[count].i = count;
    ((node *)a)[count++].tid = pthread_self();
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{
    node *p = (node *)malloc(sizeof(node) * 100);
    pthread_mutex_init(&mutex, NULL);
    thread_pool_init();
    for (int i = 0; i < 100; i++)
    {

        thread_pool_add_task(setmem, (void *)p);
    }
    sleep(2);
    for (int i = 0; i < 100; i++)
    {
        printf("%d: %d %ld\n", i, p[i].i, p[i].tid);
    }

    return 0;
}
