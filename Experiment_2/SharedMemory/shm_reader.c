#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#define SHM_SIZE 512
#define PJT_ID 114

#define SEM_READ 0 // 用于控制reader是否可以读取book
#define SEM_WRITE 1 // 用于控制writer是否可以写入book

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main()
{
    struct sembuf sem_buf;
     key_t a = ftok(".", PJT_ID);
    // 创建信号量
    int sem_id = semget(a, 2, 0666);
    if (sem_id < 0)
    {
        perror("semget_error");
        exit(EXIT_FAILURE);
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


    while (strcmp(book, "END") != 0)
    {
        printf("Trying to understand the book ...\n");
        sleep(3);
        printf("reader: %s\n", book);
        // 完成读取，允许writer写入book
        sem_buf.sem_num = SEM_READ;
        sem_buf.sem_op = 1; // V操作
        sem_buf.sem_flg = SEM_UNDO;
        if (semop(sem_id, &sem_buf, 1) == -1) {
            perror("semop");
            exit(1);
        }
        // 等待writer修改book
        sem_buf.sem_num = SEM_WRITE;
        sem_buf.sem_op = -1; // P操作
        sem_buf.sem_flg = SEM_UNDO;
        if (semop(sem_id, &sem_buf, 1) == -1) {
            perror("semop");
            exit(1);
        }
    }
}