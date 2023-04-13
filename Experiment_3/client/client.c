#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 1024
#define SERV_PORT 8081

int main(int argc, char **argv) 
{
    int sockfd, n;  
    struct sockaddr_in serv_add;
    char task[MAXLINE]; 
    char recv_line[MAXLINE + 1];

    // 计算任务输入格式：“x*y""x-y”""x+y""x/y"。数据类型为整型，以空格为分隔符
    // 例如：1+2 3*4 5/6 7-8
    printf("Please input the task: ");
    fgets(task, MAXLINE, stdin);
    task[strlen(task) - 1] = 0;

    // 创建套接字
    // printf("Please input the IP address of the server: ");
    // char ip[20];
    // scanf("%s", ip);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_add, 0, sizeof(serv_add));
    serv_add.sin_family = AF_INET;
    serv_add.sin_port = htons(8081);
    inet_pton(AF_INET, "127.0.0.1", &serv_add.sin_addr);//将点分十进制IP地址转换为网络字节序的二进制IP地址


    if(connect(sockfd, (struct sockaddr*)&serv_add, sizeof(serv_add)) < 0) {
        perror("connect error");
        exit(1);
    }

    // 发送任务
    write(sockfd, task, strlen(task));

    // 接收结果
    while((n = read(sockfd, recv_line, MAXLINE)) > 0) {
        recv_line[n] = 0;
        if(fputs(recv_line, stdout) == EOF) {
            perror("fputs error");
            exit(1);
        }
    }

    if(n < 0) {
        perror("read error");
        exit(1);
    }
    exit(0);
}
