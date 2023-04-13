#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define SNTC_SIZE 128
#define PJT_ID 114

#define SEM_WRITE 0 // 用于控制writer是否可以写入book

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

typedef struct node
{
    char sentence[SNTC_SIZE];
    int nextid;
} node;


int main()
{
    key_t a = ftok(".", PJT_ID);
    // 创建信号量
    int sem_id = semget(a, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id < 0)
    {
        perror("semget_error");
        exit(EXIT_FAILURE);
    }

    // 初始化信号量
    union semun sem_arg;
    struct sembuf sem_buf;
    sem_arg.val = 1;
    if (semctl(sem_id, SEM_WRITE, SETVAL, sem_arg) == -1)
    {
        perror("semctl");
        exit(1);
    }

    // 创建共享书
    int shb_id = shmget(a, sizeof(node), IPC_CREAT | 0666);
    if (shb_id < 0)
    {
        perror("shmget_error");
        exit(EXIT_FAILURE);
    }

    // 共享书的内容
    node *book = (node *)shmat(shb_id, NULL, 0);
    if (book == (node *)-1)
    {
        perror("shmat_error");
        exit(EXIT_FAILURE);
    }

    // 初始化书，写入两句话
    memset(book->sentence, '\0', SNTC_SIZE); // 初始化共享内存为全零字符串
    char buff[SNTC_SIZE];
    int newSentenceID = shmget(NULL, sizeof(node), IPC_CREAT | 0666);
    book->nextid = newSentenceID;
    book = (node *)shmat(newSentenceID, NULL, 0);
    strcpy(book->sentence,"how are you today?");
    book->nextid=NULL;

    while (getchar())
    {
        printf("Waitting for reader ...\n");
        // 等待reader读取完成
        sem_buf.sem_num = SEM_WRITE;
        sem_buf.sem_op = -1; // P操作
        sem_buf.sem_flg = SEM_UNDO;
        if (semop(sem_id, &sem_buf, 1) == -1)
        {
            perror("semop");
            exit(1);
        }
        // 开始输入
        printf("You can append the book now.\n");
        while (1)
        {
            fgets(buff, SNTC_SIZE, stdin);
            buff[strlen(buff) - 1] = '\0';
            if(strcmp(buff,"AP_END")==0){
                break;
            }
            else if(strcmp(buff,"END")==0){
                // 删除信号量
                semctl(sem_id, 0, IPC_RMID, sem_arg);
                exit(0);
            }
            // 追加链表结点
            newSentenceID = shmget(NULL, sizeof(node), IPC_CREAT | 0666);
            book->nextid = newSentenceID;
            book = (node *)shmat(newSentenceID, NULL, 0);
            book->nextid = NULL;
            memcpy(&(book->sentence), buff, SNTC_SIZE);
        }
        printf("Successfully writen.\n");

        // 完成修改，通知reader读取
        sem_buf.sem_num = SEM_WRITE;
        sem_buf.sem_op = 1; // V操作
        sem_buf.sem_flg = SEM_UNDO;
        if (semop(sem_id, &sem_buf, 1) == -1)
        {
            perror("semop");
            exit(1);
        }
    }

    return 0;
}