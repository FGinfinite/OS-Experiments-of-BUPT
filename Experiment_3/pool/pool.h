#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

/*程序只需要向线程池传递任务，线程池便会自动地调度这些任务。

线程池会单独使用一个线程来运行。每当程序需要添加任务时，就通过thread_pool_add_task(task , void* argv)来向线程池加入任务。

thread_pool_add_task会将对应的参数放入任务队列之中，释放一个signal，唤醒如果处于挂起状态的线程池。

线程池线程在不阻塞时会不断地从任务队列中取出任务，并且从线程池中找到空闲线程交予其执行。如果没有空闲线程，则挂起直到被发送信号告知有空闲线程了。

如果有空闲线程，那么它会把任务放到指定的位置，供空闲线程获取。被唤醒的空闲线程会取走任务，然后执行它。

*/

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