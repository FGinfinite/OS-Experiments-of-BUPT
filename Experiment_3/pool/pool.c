#include "pool.h"
#include <stdlib.h>

thread_pool *pool;

void thread_pool_init()
{
    pool = (thread_pool *)malloc(sizeof(thread_pool));
    for (int i = 0; i < MAX; i++)
    {
        pool->task_queue.task_q[i] = (task *)malloc(sizeof(task));
    }
    pool->task_buff_flag = true;
    pool->task_queue.head = 0;
    pool->task_queue.tail = 0;
    pool->task_queue.size = 0;
    pool->task_queue.max_size = MAX;
    sem_init(&pool->queue_size, 0, 0);
    sem_init(&pool->free_thread, 0, MAX);
    pthread_mutex_init(&pool->task_queue.using_queue, NULL);
    pthread_mutex_init(&pool->free_thread_mutex, NULL);
    pthread_cond_init(&pool->task_done_cond, NULL);
    pthread_cond_init(&pool->task_buff_cond, NULL);
    pthread_cond_init(&pool->wait_task, NULL);
    for (int i = 0; i < MAX; i++)
    {
        pthread_create(&pool->threads[i], NULL, thread_do, (void *)pool);
    }
    pthread_create(&pool->thread_main, NULL, thread_pool_main, (void *)pool);
}

void thread_pool_add_task(void *(*callback_function)(void *arg), void *arg)
{
    pthread_mutex_lock(&pool->task_queue.using_queue);
    // 如果队列满了，等待
    while (pool->task_queue.size == pool->task_queue.max_size)
    {
        pthread_cond_wait(&pool->task_done_cond, &pool->task_queue.using_queue);
    }
    pool->task_queue.task_q[pool->task_queue.tail]->callback_function = callback_function;
    pool->task_queue.task_q[pool->task_queue.tail]->arg = arg;
    pool->task_queue.tail = (pool->task_queue.tail + 1) % MAX;
    pool->task_queue.size++;
    pthread_mutex_unlock(&pool->task_queue.using_queue);
    sem_post(&pool->queue_size);
}

void *thread_pool_main(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&pool->free_thread_mutex);
        // 在队列为空时，挂起线程，直到有新任务加入
        // while (pool->task_queue.size == 0)
        // {
        //     // 已知BUG：当thread_pool_main检测到任务队列size为0时，它会进入本循环。但如果此时thread_pool_add_task将任务迅速填满，在该线程挂起之前，那么它将永远挂起，因为没有线程会发出task_done_cond信号。可以通过sleep(3)来明显地触发这个BUG。
        //     // sleep(3);
        //     printf("task queue is empty, waiting for new task\n");
        //     pthread_cond_wait(&pool->new_task_cond, &pool->free_thread_mutex);
        // }
        sem_wait(&pool->queue_size);
        // 如果没有空闲线程则等待
        sem_wait(&pool->free_thread);
        // 将任务放入task_buff缓冲区，等待直到它被取走
        pthread_mutex_lock(&pool->task_queue.using_queue);
        pool->task_buff.callback_function = pool->task_queue.task_q[pool->task_queue.head]->callback_function;
        pool->task_buff.arg = pool->task_queue.task_q[pool->task_queue.head]->arg;
        pool->task_queue.head = (pool->task_queue.head + 1) % MAX;
        pool->task_queue.size--;
        pthread_mutex_unlock(&pool->task_queue.using_queue);

        pool->task_buff_flag = false;
        pthread_cond_signal(&pool->wait_task);
        while (pool->task_buff_flag == false)
        {
            pthread_cond_wait(&pool->task_buff_cond, &pool->free_thread_mutex);
        }
        pthread_mutex_unlock(&pool->free_thread_mutex);
    }
}

void *thread_do(void *arg)
{
    pthread_mutex_t mutex;
    task running;
    pthread_mutex_init(&mutex, NULL);
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (pool->task_buff_flag == true)
        {
            pthread_cond_wait(&pool->wait_task, &mutex);
        }
        running.callback_function = pool->task_buff.callback_function;
        running.arg = pool->task_buff.arg;
        pool->task_buff_flag = true;
        pthread_cond_signal(&pool->task_buff_cond);
        pthread_mutex_unlock(&mutex);
        running.callback_function(running.arg);
        pthread_cond_signal(&pool->task_done_cond);
        sem_post(&pool->free_thread);
    }
}
