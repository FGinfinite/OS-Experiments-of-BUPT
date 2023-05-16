#include "sched.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define timeSlice 10

void printSentence(void *arg, int *ticks, ProcessState *flag)
{
    char *sentence = (char *)arg;
    int i;
    // 如果是初次运行则先申请空间以便保存上下文
    if (ss->running->context.data == NULL)
    {
        // 申请空间，0为i
        ss->running->context.data = (void *)malloc(sizeof(int));
        i = 0;
    }
    else
    {
        // 如果不是初次运行则恢复上下文
        i = *(int *)(ss->running->context.data);
    }
    switch (ss->running->context.pc)
    {
    case 0:
        while (sentence[i] != '\0')
        {
            printf("%c", sentence[i]);
            i++;
            (*ticks)++; // 记录CPU消耗
            needSchedule(ss);
        }
        ss->running->context.pc++;
    case 1:
        printf("\n");
        (*ticks)++;
        ss->running->context.pc++;
        needSchedule(ss);
    case 2:
        break;
    }
    (*flag) = TASK_EXITED;
}

void sum(void *arg, int *ticks, ProcessState *flag)
{
    int total, i;
    // 如果是初次运行则先申请空间以便保存上下文
    if (ss->running->context.data == NULL)
    {
        // 申请空间，0为total，1为i
        ss->running->context.data = (void *)malloc(sizeof(int) * 2);
        total = 0;
        i = 0;
    }
    else
    {
        // 如果不是初次运行则恢复上下文
        int *data = (int *)(ss->running->context.data);
        total = data[0];
        i = data[1];
    }

    int n = *(int *)arg;

    switch (ss->running->context.pc)
    {
    case 0:
        while (i <= n)
        {
            total += i;
            ++i;
            printf("sum = %d\n", total);

            (*ticks)++; // 记录CPU消耗
            totalTicks++;
            needSchedule(ss);
        }
        ss->running->context.pc++;
    case 1:
        printf("sum = %d\n", total);
        (*ticks)++;
        totalTicks++;
        ss->running->context.pc++;
        needSchedule(ss);
    case 2:
        break;
    }

    (*flag) = TASK_EXITED;
}

void halt(void *arg, int *ticks, ProcessState *flag)
{
    usleep(100);
    // printf("halt\n");
    totalTicks++;
    (*ticks)++; // Todo: 记录CPU消耗
}

void initProcess(Process *p)
{
    // 随机生成pid
    p->id = rand() % 1000;
    p->context.data = NULL;
    p->context.pc = 0;
    p->prio = DEFALT_PRIO;
    p->nice = DEFALT_NICE;
    p->state = TASK_READY;
    p->ticks = 0;
    p->vruntime = 0;
    p->runtime = 0;
    p->prevTime = 0;
}

ProcessState runProcess(Process *p)
{
    ProcessState flag = TASK_RUNNING;
    if (p->state == TASK_READY)
    {
        p->state = TASK_RUNNING;
        p->task.fun(p->task.arg, &(p->ticks), &flag);
        if (flag == TASK_EXITED)
            p->state = TASK_EXITED;
        else if (flag == TASK_HANGING)
            p->state = TASK_HANGING;
    }
    return flag;
}

void runCurrentTask(Scheduler *s)
{
    if (s->running == NULL)
        return;
    ProcessState flag = runProcess(s->running);
    // 如果是自然结束，那么就还需要再显式地调度一次
    if (s->running->task.fun == halt || (s->running->state) == TASK_EXITED)
    {
        schedule(s);
    }
}

void initScheduler(Scheduler *s)
{
    s->policy = SCHED_FCFS;
    s->running = NULL;
    s->readyQueues = (MultiQueue *)malloc(sizeof(MultiQueue));
    initMultiQueue(s->readyQueues);
    s->waitQueue = (Queue *)malloc(sizeof(Queue));
    initQueue(s->waitQueue);
}

void initMultiQueue(MultiQueue *mq)
{
    mq->size = 0;
    mq->queue = NULL;
}

void initQueue(Queue *q)
{
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void queuePush(Queue *q, Process *p)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->p = p;
    node->next = NULL;
    if (q->size == 0)
    {
        q->head = node;
        q->tail = node;
    }
    else
    {
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;
}

Process *queueFront(Queue *q)
{
    if (q->size == 0)
        return NULL;
    return q->head->p;
}

void queuePop(Queue *q)
{
    if (q->size == 0)
        return;
    Node *node = q->head;
    q->head = q->head->next;
    free(node);
    q->size--;
}

void multiQueueAddQueue(MultiQueue *mq)
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    initQueue(queue);
    mq->queue = (Queue **)realloc(mq->queue, sizeof(Queue *) * (mq->size + 1));
    mq->queue[mq->size] = queue;
    mq->size++;
}

void schedule(Scheduler *sched)
{
    sem_wait(&semReadyQueue);
    switch (sched->policy)
    {
    case SCHED_FCFS:
        // 回收僵尸进程，并且调度下一个进程运行
        if (sched->running && sched->running->state == TASK_EXITED)
        {
            recycleProcess(sched->running);
            sched->running = queueFront(sched->readyQueues->queue[0]);
            queuePop(sched->readyQueues->queue[0]);
        }
        // 似乎不应该在FCFS中有以下else过程，因为FCFS的调度只会发生在进程结束时(?)
        else
        {
            sched->running->state = TASK_READY;
            queuePush(sched->readyQueues->queue[0], sched->running);
            sched->running = queueFront(sched->readyQueues->queue[0]);
            queuePop(sched->readyQueues->queue[0]);
        }
        break;

    default:
        break;
    }
    sem_post(&semReadyQueue);
}

void needSchedule(Scheduler *sched)
{
    // 判断是否需要调度
    printf("needSchedule: ");
    switch (sched->policy)
    {
    case SCHED_FCFS:
        // 如果当前没有进程在运行或者当前进程已经结束，则调度
        if (sched->running->task.fun == halt || sched->running->state == TASK_EXITED)
        {
            printf("yes\n");
            schedule(sched);
        }
        else
        {
            printf("no\n");
        }
        break;

    default:
        break;
    }
}
/*
thread: runningCPU->runCurrentTask->runProcess->fun
->needSchedule(函数进行过程中或者执行结束后进行是否需要调度的判断)->schedule(在此处发生调度)
->(若发生调度，则fun进行中断，并且保存上下文）->runProcess->runCurrentTask->runningCPU->runCurrentTask->...
*/
void runningCPU(void *s)
{
    Scheduler *sched = (Scheduler *)s;
    printf("CPU is running...\n");
    initScheduler(sched);
    multiQueueAddQueue(sched->readyQueues);
    addHaltTask(sched); // 将空闲进程运行
    while (1)
    {
        // CPU只管运行即可，剩下的老爹来想办法
        runCurrentTask(sched);
    }
}

void console(void *s)
{
    Scheduler *sched = (Scheduler *)s;
    printf("Console is running...\n");
    while (1)
    {
        printf("1.add a task\n2. change policy.\n3.exit\n");
        int choice;
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            addTask(sched);
            break;
        case 2:
            return;
        default:
            printf("Invalid input.\n");
            break;
        }
    }
}

// Todo: 改变调度策略
void changePolicy(Scheduler *sched)
{
    printf("Input policy: 1.FCFS 2.RR 3.MLFQ 4.NORMAL(simulate CFS of Linux)\n");
    int policy;
    scanf("%d", &policy);
    sched->policy = policy - 1;
}

void addTask(Scheduler *sched)
{
    printf("addTask: Input your command.\nSUM t n: calculate the sum from 1 to n at time t.\nPRINT t sentence: print a sentence word by word at time t.\nEND: end the input.\n");
    while (1)
    {
        char buffer[128];
        // 获取整行输入
        fgets(buffer, 128, stdin);
        // 清除换行符
        buffer[strlen(buffer) - 1] = '\0';
        if(strlen(buffer) == 0)
            continue;
        char *command;
        command = strtok(buffer, " ");
        Process *p = (Process *)malloc(sizeof(Process));
        initProcess(p);
        p->task.fun = NULL;
        p->task.arg = NULL;
        if (strcmp(command, "PRINT") == 0)
        {
            command = strtok(NULL, " ");
            p->pushTIme = atoi(command);

            p->task.fun = printSentence;

            command = strtok(NULL, " ");
            p->task.arg = (void *)malloc(sizeof(char) * (strlen(command) + 1));
            strcpy(p->task.arg, buffer);
        }
        else if (strcmp(command, "SUM") == 0)
        {
            command = strtok(NULL, " ");
            p->pushTIme = atoi(command);

            p->task.fun = sum;

            command = strtok(NULL, " ");
            int *n = (int *)malloc(sizeof(int));
            *n = atoi(command);
            p->task.arg = (void *)n;
        }
        else if (strcmp(command, "END") == 0)
        {
            free(p);
            break;
        }
        else
        {
            printf("Invalid input.\n");
            free(p);
            continue;
        }
        sem_wait(&semWaitQueue);
        queuePush(sched->waitQueue, p);
        sem_post(&semWaitQueue);
    }
}

void addHaltTask(Scheduler *sched)
{
    Process *p = (Process *)malloc(sizeof(Process));
    p->id = 0;
    p->prio = 0;
    p->nice = 0;
    p->state = TASK_READY;
    p->task.fun = halt;
    p->task.arg = NULL;
    p->ticks = 0;
    p->vruntime = 0;
    p->runtime = 0;
    p->prevTime = 0;
    sched->running = p;
}

void recycleProcess(Process *p)
{
    free(p->task.arg);
    free(p->context.data);
    free(p);
}

void jobSchedule(void *s)
{
    Scheduler *sched = (Scheduler *)s;
    while (1)
    {
        sem_wait(&semWaitQueue);
        // 检查等待队列中是否有进程可以运行(此处将队列当作链表处理)
        if (sched->waitQueue->size != 0)
        {
            Node *t = sched->waitQueue->head;
            Node *prev = NULL;
            while (t != NULL)
            {
                if (t->p->pushTIme <= totalTicks)
                {
                    sem_wait(&semReadyQueue);
                    queuePush(sched->readyQueues->queue[0], t->p);
                    sem_post(&semReadyQueue);
                    // 删除此结点
                    if (prev == NULL)
                    {
                        sched->waitQueue->head = t->next;
                    }
                    else
                    {
                        prev->next = t->next;
                    }
                    free(t);
                    if (prev != NULL)
                        t = prev->next;
                    else
                        t=sched->waitQueue->head;
                    sched->waitQueue->size--;
                }
                else
                {

                    prev = t;
                    t = t->next;
                }
            }
        }
        sem_post(&semWaitQueue);
        usleep(500); // 每间隔5us检查一次
    }
}

int main()
{
    sem_init(&semReadyQueue, 0, 1);
    sem_init(&semWaitQueue, 0, 1);
    Scheduler *sched = (Scheduler *)malloc(sizeof(Scheduler));
    ss = sched;
    pthread_t threadCPU, threadConsole, threadJobSchedule;
    pthread_create(&threadCPU, NULL, (void *)runningCPU, (void *)sched);
    pthread_create(&threadConsole, NULL, (void *)console, (void *)sched);
    pthread_create(&threadJobSchedule, NULL, (void *)jobSchedule, (void *)sched);
    pthread_join(threadCPU, NULL);
    pthread_join(threadConsole, NULL);
    pthread_join(threadJobSchedule, NULL);
    return 0;
}
