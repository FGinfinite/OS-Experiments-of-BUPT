#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define SHM_SIZE 512
#define PJT_ID 114

#define SEM_READ 0  // 用于控制reader是否可以读取book
#define SEM_WRITE 1 // 用于控制writer是否可以写入book

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main()
{
    key_t a = ftok(".", PJT_ID);
    // 创建信号量
    int sem_id = semget(a, 2, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id < 0)
    {
        perror("semget_error");
        exit(EXIT_FAILURE);
    }

    // 初始化信号量
    union semun sem_arg;
    struct sembuf sem_buf;
    sem_arg.val = 0;
    if (semctl(sem_id, SEM_READ, SETVAL, sem_arg) == -1)
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
    sem_arg.val = 0;
    if (semctl(sem_id, SEM_WRITE, SETVAL, sem_arg) == -1)
    {
        perror("semctl");
        exit(1);
    }

    // 创建共享书
    int shb_id = shmget(a, SHM_SIZE, IPC_CREAT | 0666);
    if (shb_id < 0)
    {
        perror("shmget_error");
        exit(EXIT_FAILURE);
    }

    // 共享书的内容
    char *book = (char *)shmat(shb_id, NULL, 0);
    if (book == (char *)-1)
    {
        perror("shmat_error");
        exit(EXIT_FAILURE);
    }

    // 初始化书
    memset(book, '\0', SHM_SIZE); // 初始化共享内存为全零字符串
    strcpy(book,"What do you like?");

    char buff[SHM_SIZE];
    while (1)
    {
        printf("Waitting for reader ...\n");
        // 等待reader读取完成
        sem_buf.sem_num = SEM_READ;
        sem_buf.sem_op = -1; // P操作
        sem_buf.sem_flg = SEM_UNDO;
        if (semop(sem_id, &sem_buf, 1) == -1)
        {
            perror("semop");
            exit(1);
        }
        
        printf("You can rewrite the book now.\n");
        fgets(buff, SHM_SIZE, stdin);
        fflush(stdin);
        buff[strlen(buff) - 1] = '\0';
        memcpy(book, buff, SHM_SIZE);
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

        if(strcmp(book, "END")==0){
            break;
        }
    }

    // 删除信号量
    if (semctl(sem_id, 0, IPC_RMID, sem_arg) == -1)
    {
        perror("semctl");
        exit(1);
    }

    return 0;
}