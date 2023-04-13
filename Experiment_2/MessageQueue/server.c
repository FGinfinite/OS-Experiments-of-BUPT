#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SVKEY       75  // 消息队列关键字
#define REQ_TYPE    1   // 请求消息类型

struct message {
    long mtype;     // 消息类型
    pid_t sender;   // 发送者的进程标识
};

int main() {
    int msgqid;
    struct message msg;

    printf("Here server is.\n");
    // 创建消息队列
    msgqid = msgget(SVKEY, IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // 等待请求消息
        if (msgrcv(msgqid, &msg, sizeof(struct message) - sizeof(long), REQ_TYPE, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        // 显示正在为client进程服务
        printf("serving for client %d\n", msg.sender);

        // 向client进程发送应答消息
        msg.mtype = msg.sender;
        msg.sender = getpid();
        if (msgsnd(msgqid, &msg, sizeof(struct message) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        printf("Sent the message.\n");
    }

    exit(EXIT_SUCCESS);
}
