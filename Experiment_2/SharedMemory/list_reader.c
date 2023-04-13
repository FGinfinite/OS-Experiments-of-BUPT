#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>

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
    struct sembuf sem_buf;
    key_t a = ftok(".", PJT_ID);
    // 创建信号量
    int sem_id = semget(a, 1, 0666);
    if (sem_id < 0)
    {
        perror("semget_error");
        exit(EXIT_FAILURE);
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

    while (strcmp(book->sentence, "END") != 0)
    {

        // 等待writer修改book
        sem_buf.sem_num = SEM_WRITE;
        sem_buf.sem_op = -1; // P操作
        sem_buf.sem_flg = SEM_UNDO;
        if (semop(sem_id, &sem_buf, 1) == -1)
        {
            perror("semop");
            exit(1);
        }

        printf("Trying to understand the book ...\n");
        sleep(3);
        if (book->nextid == 0)
        {
            sem_buf.sem_num = SEM_WRITE;
            sem_buf.sem_op = 1; // V操作
            sem_buf.sem_flg = SEM_UNDO;
            if (semop(sem_id, &sem_buf, 1) == -1)
            {
                perror("semop");
                exit(1);
            }
            exit(0);
        }
        else
        {
            book = (node *)shmat(book->nextid, NULL, 0);
        }
        printf("reader: %s\n", book->sentence);
        // 完成读取，允许writer写入book
        sem_buf.sem_num = SEM_WRITE;
        sem_buf.sem_op = 1; // V操作
        sem_buf.sem_flg = SEM_UNDO;
        if (semop(sem_id, &sem_buf, 1) == -1)
        {
            perror("semop");
            exit(1);
        }
    }
}