#include "MsgQueueAPI.h"
#include <sys/shm.h>
#include <stdbool.h>
#include <stddef.h>
#define MYKEY 1145
// 获取队列
key_t msgget(key_t key, int msgflg)
{
    int shmid = shmget(MYKEY, sizeof(bool) + sizeof(key_t), IPC_CREAT | 0666);
    void *shm = shmat(shmid, NULL, 0);
    *(bool *)shm = true;
    *(key_t *)(shm + sizeof(bool)) = key;
    return key;
}
// 发送信息
int msgsnd(int msgid,const void* msg,size_t msgsz,int msgflg){
    
}