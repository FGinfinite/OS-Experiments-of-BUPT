#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "pool.h"

#define MAXLINE 1024
#define SERV_PORT 8081

void *handle_connection(void *arg);
void *handle_task(void *arg);
// 将空格符改为0，返回下一个字符的指针
char *find_space(char *source);
// 返回操作符的指针
char *find_operator(char *source);
// 将字符串转换为整型，搜索直到字符串中的第一个非数字字符
int str_to_int(char *source);
// 读取用户输入，如果是‘n’，则关闭服务器，如果是‘y’，则继续等待连接
void *read_choice(void *choice);

typedef struct
{
    pthread_mutex_t *mutex;
    pthread_cond_t *available;
    char *results;
    char *task;
} data_of_thread;

struct sockaddr_in serv_add, cli_add;

int main(int argc, char **argv)
{
    int sockfd; // 分别代表监听套接字和已连接套接字
    socklen_t cli_len;

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);     // 创建套接字
    bzero(&serv_add, sizeof(serv_add));           // 初始化服务器地址
    serv_add.sin_family = AF_INET;                // 设置地址族为IPv4
    serv_add.sin_addr.s_addr = htonl(INADDR_ANY); // 设置IP地址为本地任意IP
    serv_add.sin_port = htons(SERV_PORT);         // 设置端口号
    // 设置端口复用
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // 设置超时时间为一秒
    struct timeval tv = {1, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // 绑定套接字
    bind(sockfd, (struct sockaddr *)&serv_add, sizeof(serv_add)); // 绑定套接字
    listen(sockfd, 1024);                                         // 监听套接字

    // 创建线程池
    thread_pool_init();

    // 创建线程等待用户对服务器的指令
    char choice = 'y';
    thread_pool_add_task(read_choice, (void *)&choice);

    // 等待连接
    printf("Waiting for connection...\n");
    while (choice == 'y')
    {
        cli_len = sizeof(cli_add);
        int connfd = accept(sockfd, (struct sockaddr *)&cli_add, &cli_len);
        if (connfd >= 0)
        {
            printf("Accept a new connection.\n");
            thread_pool_add_task(handle_connection, (void *)&connfd);
        }
    }

    // 关闭套接字
    close(sockfd);
    return 0;
}

void *handle_connection(void *arg)
{
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t available = PTHREAD_COND_INITIALIZER;
    char results[MAXLINE] = {0};
    int count = 0, total = 0;

    int connfd = *(int *)arg;
    char buf[MAXLINE];
    printf("Connection from %s, port %d\n", inet_ntop(AF_INET, &cli_add.sin_addr, buf, sizeof(buf)), ntohs(cli_add.sin_port));

    // 接收任务
    int n = read(connfd, buf, MAXLINE);
    memset(buf + n, 0, MAXLINE - n);

    // 分割计算任务
    char *task = buf;
    char *next = find_space(buf);
    while (task != NULL)
    {
        total++;
        // 经测试，如果是在这里创建新的argv，那么新线程中的argv的值会被覆盖，导致结果错误
        // 共享锁，条件变量以及结果数组，但是task是线程私有的
        // data_of_thread arg;
        // arg.task = task;
        // arg.mutex = &lock;
        // arg.available = &available;
        // arg.results = results;
        data_of_thread *argv = (data_of_thread *)malloc(sizeof(data_of_thread));
        argv->task = task;
        argv->mutex = &lock;
        argv->available = &available;
        argv->results = results;
        // 创建新线程
        thread_pool_add_task(handle_task, (void *)argv);
        task = next;
        next = find_space(next);
    }

    // 等待所有线程计算结束
    pthread_mutex_lock(&lock);
    while (count < total)
    {
        pthread_cond_wait(&available, &lock);
        count++;
    }
    pthread_mutex_unlock(&lock);
    printf("All tasks are finished, the result is\n%s", results);
    // 发送结果
    write(connfd, results, strlen(results));
    close(connfd);
    // 结束线程
    pthread_exit(NULL);
}

void *handle_task(void *arg)
{
    // data->task="3*4"
    int pid = pthread_self();
    data_of_thread *data = (data_of_thread *)arg;
    printf("Thread %d is handling the task %s\n", (int)pthread_self(), data->task);
    int result = 0;
    // 寻找运算符
    char *op = find_operator(data->task);
    // 获取操作数
    int a = str_to_int(data->task);
    int b = str_to_int(op + 1);
    // 计算结果
    switch (*op)
    {
    case '+':
        result = a + b;
        break;
    case '-':
        result = a - b;
        break;
    case '*':
        result = a * b;
        break;
    case '/':
        result = a / b;
        break;
    }
    // 挂起2～7秒，模拟计算过程
    sleep(rand() % 5 + 2);
    // 互斥地将结果拼接results后
    pthread_mutex_lock(data->mutex);
    char result_str[128];
    sprintf(result_str, "%d %c %d = %d\n", a, *op, b, result);
    strcat(data->results, result_str);
    pthread_mutex_unlock(data->mutex);
    printf("%d %c %d = %d\n", a, *op, b, result);
    // 通知connection线程
    pthread_cond_signal(data->available);
    // 释放内存
    free(data);
    // 结束线程
    pthread_exit(NULL);
}

char *find_space(char *source)
{
    if (source == NULL)
        return NULL;
    int i = 0;
    for (; i < strlen(source); i++)
    {
        if (source[i] == ' ')
        {
            source[i] = 0;
            return source + i + 1;
        }
    }
    return NULL;
}

char *find_operator(char *source)
{
    if (source == NULL)
        return NULL;
    int i = 0;
    for (; i < strlen(source); i++)
    {
        if (source[i] == '+' || source[i] == '-' || source[i] == '*' || source[i] == '/')
        {
            return source + i;
        }
    }
    return NULL;
}

int str_to_int(char *source)
{
    int result = 0;
    int i = 0;
    for (; i < strlen(source); i++)
    {
        if (source[i] >= '0' && source[i] <= '9')
        {
            result = result * 10 + source[i] - '0';
        }
        else
        {
            break;
        }
    }
    return result;
}

void *read_choice(void *choice)
{
    while (*(char *)choice != 'n')
    {
        printf("You can input 'n' to stop the server.\n");
        scanf("%c", (char *)choice);
    }
}