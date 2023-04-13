#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

#define MAX 10

typedef struct
{
    void *(*callback_function)(void *arg);
    void *arg;
} task;

typedef struct
{
    task *task_q[MAX]; // 任务队列
    int head;
    int tail;
    int size;
    int max_size;
    pthread_mutex_t using_queue;
} queue;

typedef struct
{
    pthread_mutex_t free_thread_mutex;

    pthread_cond_t task_done_cond;
    pthread_cond_t task_buff_cond;
    pthread_cond_t wait_task;
    pthread_t threads[MAX];
    pthread_t thread_main;
    queue task_queue;
    task task_buff;
    sem_t queue_size;
    sem_t free_thread;
    bool task_buff_flag;
} thread_pool;

// 实验项目中只对这个线程池进行操作

// 初始化一个大小为10的线程池
void thread_pool_init();
// 向线程池中添加任务
void thread_pool_add_task(void *(*callback_function)(void *arg), void *arg);
// 线程池主函数
void *thread_pool_main(void *arg);
// 线程池中的线程执行的函数
void *thread_do(void *arg);