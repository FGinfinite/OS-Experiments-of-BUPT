#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>

void *thread2(void *arg)
{
    int *i = (int *)arg;
    sleep(3);
    printf("thread2: i=%d\n", *i);
    *i = 99;
    printf("thread2: i=%d after changed\n", *i);
    pthread_exit(0);
}
void *thread1()
{
    for (int i = 0; i < 10; i++)
    {
        pthread_t p2;
        int j = i;
        pthread_create(&p2, NULL, thread2, &j);
        // pthread_join(p2, NULL);
        printf("In circle of %d thread1: i=%d after changed\n", i, j);
    }
    sleep(30);
    pthread_exit(0);
}

int main()
{
    int i = 3;
    pthread_t p1;
    pthread_create(&p1, NULL, thread1, NULL);
    printf("main: i=%d\n", i);
    pthread_join(p1, NULL);
    return 0;
}
