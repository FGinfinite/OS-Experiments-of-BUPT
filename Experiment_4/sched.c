#include "sched.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void printSentence(void *arg, int *ticks, ProcessState *flag)
{
    Process *p = ss->running;
    char *sentence = (char *)arg;
    int i;
    // 如果是初次运行则先申请空间以便保存上下文
    if (p->context.data == NULL)
    {
        // 申请空间，0为i
        p->context.data = (void *)malloc(sizeof(int));
        i = 0;
    }
    else
    {
        // 如果不是初次运行则恢复上下文
        i = *(int *)(p->context.data);
    }

    switch (p->context.pc)
    {
    case 0:
        while (sentence[i] != '\0')
        {
            printf("PRINT: %c\n", sentence[i]);
            i++;
            (*ticks)++; // 记录CPU消耗
            totalTicks++;
            if ((*flag = needSchedule(ss)) == TASK_READY)
            {
                // 保存上下文
                *(int *)(p->context.data) = i;
                return;
            }
        }
        p->context.pc++;
    case 1:
        printf("PRINT: Finished printing sentence.\n");
        (*ticks)++;
        p->context.pc++;
        totalTicks++;
        if ((*flag = needSchedule(ss)) == TASK_READY)
        {
            // 保存上下文
            *(int *)(p->context.data) = i;
            return;
        }
    case 2:
        break;
    }
    (*flag) = TASK_EXITED;
}

void sum(void *arg, int *ticks, ProcessState *flag)
{
    Process *p = ss->running;
    int total, i;
    // 如果是初次运行则先申请空间以便保存上下文
    if (p->context.data == NULL)
    {
        // 申请空间，0为total，1为i
        p->context.data = (void *)malloc(sizeof(int) * 2);
        total = 0;
        i = 0;
    }
    else
    {
        // 如果不是初次运行则恢复上下文
        int *data = (int *)(p->context.data);
        total = data[0];
        i = data[1];
    }

    int n = *(int *)arg;

    switch (p->context.pc)
    {
    case 0:
        while (i <= n)
        {
            total += i;
            ++i;
            printf("SUM: sum = %d\n", total);

            (*ticks)++; // 记录CPU消耗
            totalTicks++;
            if ((*flag = needSchedule(ss)) == TASK_READY)
            {
                // 保存上下文
                ((int *)p->context.data)[0] = total;
                ((int *)p->context.data)[1] = i;
                return;
            }
        }
        p->context.pc++;
    case 1:
        printf("SUM: total sum = %d\n", total);

        (*ticks)++;
        totalTicks++;
        p->context.pc++;
        if ((*flag = needSchedule(ss)) == TASK_READY)
        {
            // 保存上下文
            ((int *)p->context.data)[0] = total;
            ((int *)p->context.data)[1] = i;
            return;
        }
    case 2:
        break;
    }

    (*flag) = TASK_EXITED;
}

void setBigInt(BigInt *bi, int value)
{
    bi->head = (BigIntNode *)malloc(sizeof(BigIntNode));
    BigIntNode *node = bi->head;
    BigIntNode *prev = node;
    node->data = value % 10;
    value /= 10;
    node->next = NULL;
    node->prev = NULL;
    while (value)
    {
        node = (BigIntNode *)malloc(sizeof(BigIntNode));
        node->data = value % 10;
        value /= 10;
        node->next = NULL;
        prev->next = node;
        node->prev = prev;
        prev = node;
    }
}

void factorial(void *arg, int *ticks, ProcessState *flag)
{

    Process *p = ss->running;
    BigInt total;
    int count;
    int carry;
    BigIntNode *node;
    BigIntNode *prev;
    // 如果是初次运行则先申请空间以便保存上下文
    if (p->context.data == NULL)
    {
        // 申请空间
        p->context.data = (void *)malloc(sizeof(int) * 2 + sizeof(BigIntNode *) * 2 + sizeof(BigInt));
        setBigInt(&total, 1);
        count = 1;
        carry = 0;
        node = total.head;
        prev = NULL;
    }
    else
    {
        // 如果不是初次运行则恢复上下文
        count = *(int *)(p->context.data);
        carry = *((int *)(p->context.data) + sizeof(int));
        node = *((BigIntNode **)(p->context.data) + sizeof(int) * 2);
        prev = *((BigIntNode **)(p->context.data) + sizeof(int) * 2 + sizeof(BigIntNode *));
        total = *((BigInt *)(p->context.data) + sizeof(int) * 2 + sizeof(BigIntNode *) * 2);
    }

    int n = *(int *)arg;

    switch (p->context.pc < 2)
    {
    case 1:
        while (count <= n)
        {
            printf("FACTORIAL: count = %d\n", count);
            node = total.head;
            prev = NULL;
            switch (p->context.pc)
            {
            // total与count相乘
            case 0:
                while (node)
                {
                    int temp = node->data * count + carry;
                    node->data = temp % 10;
                    carry = temp / 10;
                    prev = node;
                    node = node->next;
                }

                p->context.pc++;
                (*ticks)++;
                totalTicks++;
                if ((*flag = needSchedule(ss)) == TASK_READY)
                {
                    // 保存上下文
                    *(int *)(p->context.data) = count;
                    *((int *)(p->context.data) + sizeof(int)) = carry;
                    *((BigIntNode **)(p->context.data) + sizeof(int) * 2) = node;
                    *((BigIntNode **)(p->context.data) + sizeof(int) * 2 + sizeof(BigIntNode *)) = prev;
                    *((BigInt *)(p->context.data) + sizeof(int) * 2 + sizeof(BigIntNode *) * 2) = total;
                    return;
                }

            case 1:
                // 如果有进位则依次加到最后
                while (carry)
                {
                    node = (BigIntNode *)malloc(sizeof(BigIntNode));
                    node->data = carry % 10;
                    carry /= 10;
                    node->next = NULL;
                    node->prev = prev;
                    prev->next = node;
                    prev = node;
                }
                printf("FACTORIAL: Finished count = %d\n", count);
                count++;

                (*ticks)++;
                totalTicks++;
                p->context.pc--;
                if ((*flag = needSchedule(ss)) == TASK_READY)
                {
                    // 保存上下文
                    *(int *)(p->context.data) = count;
                    *((int *)(p->context.data) + sizeof(int)) = carry;
                    *((BigIntNode **)(p->context.data) + sizeof(int) * 2) = node;
                    *((BigIntNode **)(p->context.data) + sizeof(int) * 2 + sizeof(BigIntNode *)) = prev;
                    *((BigInt *)(p->context.data) + sizeof(int) * 2 + sizeof(BigIntNode *) * 2) = total;
                    return;
                }
            }
        }
        p->context.pc = 2;
    case 0:
        switch (p->context.pc)
        {
        case 2:
            // 从prev开始向前逐个打印
            printf("FACTORIAL: total = ");
            while (prev)
            {
                printf("%d", prev->data);
                prev = prev->prev;
            }
            printf("\n");

            (*ticks)++;
            totalTicks++;
            p->context.pc++;
            if ((*flag = needSchedule(ss)) == TASK_READY)
            {
                // 保存上下文
                *(int *)(p->context.data) = count;
                *((int *)(p->context.data) + sizeof(int)) = carry;
                *((BigIntNode **)(p->context.data) + sizeof(int) * 2) = node;
                *((BigIntNode **)(p->context.data) + sizeof(int) * 2 + sizeof(BigIntNode *)) = prev;
                *((BigInt *)(p->context.data) + sizeof(int) * 2 + sizeof(BigIntNode *) * 2) = total;
                return;
            }
        case 3:
            break;
        }
    }
    (*flag) = TASK_EXITED;
}

void idle(void *arg, int *ticks, ProcessState *flag)
{
    usleep(100);
    // printf("idle\n");
    totalTicks++;
    (*ticks)++;
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
    p->level = 0;
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
    if (s->running->task.fun == idle || (s->running->state) == TASK_EXITED)
    {
        if (s->running->task.fun != idle)
            printf("Process %d exited at ticks %d and schedule.\n", s->running->id, totalTicks);
        schedule(s);
    }
}

void initScheduler(Scheduler *s)
{
    s->timeSlice = DEFALT_TIME_SLICE;

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
    sched->running->ticks = 0; // 重置时间片
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
        else
        {
            sched->running->state = TASK_READY;
            queuePush(sched->readyQueues->queue[0], sched->running);
            sched->running = queueFront(sched->readyQueues->queue[0]);
            queuePop(sched->readyQueues->queue[0]);
        }
        break;
    case SCHED_RR_:
        if (sched->running && sched->running->state == TASK_EXITED)
        {
            recycleProcess(sched->running);
            sched->running = queueFront(sched->readyQueues->queue[0]);
            queuePop(sched->readyQueues->queue[0]);
        }
        else
        {
            sched->running->state = TASK_READY;
            queuePush(sched->readyQueues->queue[0], sched->running);
            sched->running = queueFront(sched->readyQueues->queue[0]);
            queuePop(sched->readyQueues->queue[0]);
        }
        break;
    case SCHED_MLFQ:
        // 如果当前进程已经结束，则回收进程
        if (sched->running && sched->running->state == TASK_EXITED)
        {
            recycleProcess(sched->running);
            sched->running = pickFirstProcess(sched->readyQueues);
            popFirstProcess(sched->readyQueues);
        }
        // 如果当前进程还没有结束，那么就将其放回就绪队列
        else
        {
            sched->running->state = TASK_READY;
            queuePush(sched->readyQueues->queue[sched->running->level == MAX_QUEUES - 1 ? sched->running->level : ++(sched->running->level)], sched->running);
            sched->running = pickFirstProcess(sched->readyQueues);
            popFirstProcess(sched->readyQueues);
        }
        break;

    default:
        break;
    }
    sem_post(&semReadyQueue);
}

ProcessState needSchedule(Scheduler *sched)
{
    // 判断是否需要调度
    switch (sched->policy)
    {
    case SCHED_FCFS:
        // 如果当前没有进程在运行或者当前进程已经结束，则调度
        if (sched->running->task.fun == idle || sched->running->state == TASK_EXITED)
        {
            printf("needSchedule: FCFS---yes\n");
            schedule(sched);
            return TASK_READY;
        }
        else
        {
            return TASK_RUNNING;
        }
        break;
    case SCHED_RR_:
        if (sched->running->task.fun == idle || sched->running->state == TASK_EXITED)
        {
            printf("needSchedule: RR---yes\n");
            schedule(sched);
            return TASK_READY;
        }
        else if (sched->running->ticks >= sched->timeSlice)
        {
            printf("needSchedule: RR---yes\n");
            schedule(sched);
            return TASK_READY;
        }
        else
        {
            return TASK_RUNNING;
        }
        break;
    case SCHED_MLFQ:
        if (sched->running->task.fun == idle || sched->running->state == TASK_EXITED || sched->running->ticks >= sched->MQtimeSlices[sched->running->level])
        {
            printf("needSchedule: MLFQ---yes\n");
            schedule(sched);
            return TASK_READY;
        }
        else
        {
            return TASK_RUNNING;
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

    // 初始化调度器
    initScheduler(sched);
    // 初始化多级反馈队列
    for (int i = 0; i < MAX_QUEUES; i++)
        multiQueueAddQueue(sched->readyQueues);
    sched->MQtimeSlices[0] = DEFALT_MQ_TIME_SLICE_1;
    sched->MQtimeSlices[1] = DEFALT_MQ_TIME_SLICE_2;
    sched->MQtimeSlices[2] = DEFALT_MQ_TIME_SLICE_3;
    sched->MQtimeSlices[3] = DEFALT_MQ_TIME_SLICE_4;

    addHaltTask(sched); // 将空闲进程运行
    while (1)
    {
        // CPU只管运行即可，调度器负责调度
        runCurrentTask(sched);
    }
}

void console(void *s)
{
    Scheduler *sched = (Scheduler *)s;
    printf("Console is running...\n");
    while (1)
    {
        printf("1. add a task\n2. change policy.\n3. exit\n");
        int choice;
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            addTask(sched);
            break;
        case 2:
            changePolicy(sched);
            break;
        case 3:
            exit(0);
            break;
        default:
            printf("Invalid input.\n");
            break;
        }
    }
}

// 改变调度策略
void changePolicy(Scheduler *sched)
{
    printf("Input policy: 1.FCFS 2.RR 3.MLFQ\n");
    int policy;
    scanf("%d", &policy);
    sched->policy = policy - 1;
}

void addTask(Scheduler *sched)
{
    printf("addTask: Input your command.\nSUM t n: calculate the sum from 1 to n at time t.\nPRINT t sentence: print a sentence word by word at time t.\nFACTORIAL t n: calculate the factorial of n at time t.\nEND: end the input.\n");
    while (1)
    {
        char buffer[128];
        // 获取整行输入
        fgets(buffer, 128, stdin);
        // 清除换行符
        buffer[strlen(buffer) - 1] = '\0';
        if (strlen(buffer) == 0)
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
            strcpy(p->task.arg, command);
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
        else if (strcmp(command, "FACTORIAL") == 0)
        {
            command = strtok(NULL, " ");
            p->pushTIme = atoi(command);

            p->task.fun = factorial;

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
    p->task.fun = idle;
    p->task.arg = NULL;
    p->ticks = 0;
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
                        t = sched->waitQueue->head;
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

Process *pickFirstProcess(MultiQueue *mq)
{
    for (int i = 0; i < mq->size; i++)
    {
        if (mq->queue[i]->size != 0)
        {
            return queueFront(mq->queue[i]);
        }
    }
    return NULL;
}

void popFirstProcess(MultiQueue *mq)
{
    for (int i = 0; i < mq->size; i++)
    {
        if (mq->queue[i]->size != 0)
        {
            queuePop(mq->queue[i]);
            return;
        }
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