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

    printf("Here client is.\n");
    // 创建消息队列
    msgqid = msgget(SVKEY, IPC_CREAT|0666);
    if (msgqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // 向server进程发送请求消息
    msg.mtype = REQ_TYPE;
    msg.sender = getpid();
    if (msgsnd(msgqid, &msg, sizeof(struct message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    printf("Sent the message.\n");
    // 等待应答消息
    if (msgrcv(msgqid, &msg, sizeof(struct message) - sizeof(long), getpid(), 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    // 显示接收到的server进程的标识ID
    printf("receive reply from %d\n", msg.sender);

    exit(EXIT_SUCCESS);
}
