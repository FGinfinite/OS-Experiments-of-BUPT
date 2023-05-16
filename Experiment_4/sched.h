#include <pthread.h>
#include <semaphore.h>
#define DEFALT_PRIO 120
#define DEFALT_NICE 0
typedef enum
{
    // 任务状态
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_EXITED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
} ProcessState;

typedef enum
{
    // 调度策略
    SCHED_FCFS,
    SCHED_RR_,
    SCHED_MLFQ,
    SCHED_NORMAL

} SchedPolicy;

// 任务
typedef struct
{
    // 函数指针
    void (*fun)(void *arg, int *ticks, ProcessState *flag);
    // 函数参数
    void *arg;
} Task;

typedef struct context
{
    // 指令寄存器
    int pc;
    // 进程数据
    void *data;
} Context;

typedef struct
{
    // 任务
    int id;
    Task task;
    // 任务优先级
    int prio;
    // 谦让值
    int nice;
    // 任务状态
    ProcessState state;
    // 上下文
    Context context;
    // 被加入CPU就绪队列的时间（方便批量增加进程）
    int pushTIme;
    // CPU消耗
    int ticks;
    int vruntime; // 虚拟运行时间
    int runtime;  // 运行时间
    int prevTime; // 上次运行时间
} Process;

// 队列节点
typedef struct node
{
    Process *p;
    struct node *next;
} Node;

// 队列
typedef struct
{
    Node *head;
    Node *tail;
    int size;
} Queue;

// 多级队列
typedef struct
{
    Queue **queue;
    int size;
} MultiQueue;

// 调度器
typedef struct
{
    Process *running;
    MultiQueue *readyQueues;
    // 任务会被首先加入至waitQueue中，到特定时间了才会移入readyQueues
    Queue* waitQueue;
    SchedPolicy policy;
} Scheduler;

sem_t semWaitQueue; // 等待队列必须互斥地操作
sem_t semReadyQueue; // 等待队列必须互斥地操作
int totalTicks; // 总的时钟滴答数
Scheduler *ss;  // 调度器

void schedule(Scheduler *sched);
// 打印句子
void printSentence(void *arg, int *ticks, ProcessState *flag);
// 计算1～n的和
void sum(void *arg, int *ticks, ProcessState *flag);
// 空闲进程
void halt(void *arg, int *ticks, ProcessState *flag);
void initProcess(Process *p);
// 运行调度器的当前任务
void runCurrentTask(Scheduler *sched);
// 运行进程
ProcessState runProcess(Process *p);
// 在每条指令之后都判断是否需要调度（类似轮询？）
void needSchedule(Scheduler *sched);
void initScheduler(Scheduler *s);
void initQueue(Queue *q);
void initMultiQueue(MultiQueue *mq);
void queuePush(Queue *q, Process *p);
Process *queueFront(Queue *q);
void queuePop(Queue *q);
void multiQueueAddQueue(MultiQueue *mq);
/*模拟CPU的线程。
1.如果没有任务，则应该运行halt，模拟系统空闲进程
2.实时响应控制台的指令*/
void runningCPU(void *s);
// 为调度器增加halt任务
void addHaltTask(Scheduler *sched);
// 调用此函数就会向调度器中添加一个任务
void addTask(Scheduler *sched);
// 模拟控制台的线程
void console(void *s);
// 更改调度策略
void changePolicy(Scheduler *sched);
// 回收进程
void recycleProcess(Process *p);
// 对主调度器进行作业调度的线程
void jobSchedule(void *s);
