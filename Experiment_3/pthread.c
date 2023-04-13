#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 10

typedef struct
{
    int data[BUFFER_SIZE];   // 缓冲区数组
    int readPos;             // 读的位置
    int writePos;            // 写的位置
    pthread_mutex_t lock;    // 互斥锁
    pthread_cond_t notEmpty; // 非空信号
    pthread_cond_t notFull;  // 非满信号
} Buffer;

Buffer buffer; // 缓冲区

void *consumer()
{
    int data;
    do
    {
        pthread_mutex_lock(&buffer.lock); // 加锁
        while (buffer.readPos == buffer.writePos)
        {                                                      // 缓冲区为空
            pthread_cond_wait(&buffer.notEmpty, &buffer.lock); // 等待非空信号
        }
        data = buffer.data[buffer.readPos]; // 读数据
        printf("readPos = %d\t", buffer.readPos);
        buffer.readPos = (buffer.readPos + 1) % BUFFER_SIZE; // 读指针后移
        pthread_cond_signal(&buffer.notFull);                // 发送非满信号
        pthread_mutex_unlock(&buffer.lock);                  // 解锁
        printf("Consume %d\n", data);
    } while (data != -1);
    return NULL;
}

void *producter()
{
    int data;
    int n = 0;
    do
    {
        n++;
        pthread_mutex_lock(&buffer.lock); // 加锁
        while ((buffer.writePos + 1) % BUFFER_SIZE == buffer.readPos)
        {                                                     // 缓冲区满
            pthread_cond_wait(&buffer.notFull, &buffer.lock); // 等待非满信号
        }
        data = rand() % 1100; // 随机数
        data = n == 999 ? -1 : data;
        buffer.data[buffer.writePos] = data; // 写数据
        printf("writePos = %d\t", buffer.writePos);
        buffer.writePos = (buffer.writePos + 1) % BUFFER_SIZE; // 写指针后移
        pthread_cond_signal(&buffer.notEmpty);                 // 发送非空信号
        pthread_mutex_unlock(&buffer.lock);                    // 解锁
        printf("Product %d\n", data);
        // sleep for a while
        // sleep(1);
    } while (n < 1000);
    return NULL;
}

int main()
{
    pthread_t tid1, tid2;
    buffer.readPos = 0;
    buffer.writePos = 0;
    pthread_mutex_init(&buffer.lock, NULL);
    pthread_cond_init(&buffer.notEmpty, NULL);
    pthread_cond_init(&buffer.notFull, NULL);
    pthread_create(&tid1, NULL, producter, NULL);
    pthread_create(&tid2, NULL, consumer, NULL);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    return 0;
}