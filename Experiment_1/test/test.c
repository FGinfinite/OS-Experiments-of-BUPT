#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

#define SHM_SIZE 1024
#define MAX_READERS 3

pid_t pids[MAX_READERS];
int book_id;
char *book_shared_memory;
int status_id;
bool *status_shared_memory;

void stop_readers()
{
    for (int i = 0; i < MAX_READERS; i++)
    {
        kill(pids[i], SIGSTOP);
    }
}

void continue_readers()
{
    for (int i = 0; i < MAX_READERS; i++)
    {
        kill(pids[i], SIGCONT);
    }
}

void reader_process(int reader_id)
{
    srand(time(NULL) ^ (reader_id << 16));
    int index = 0;
    while (1)
    {
        char c = book_shared_memory[index];
        if (c == '\0')
        {
            printf("Child %d process finished reading\n",reader_id);
            status_shared_memory[reader_id - 1] = true;
            break;
        }
        printf("Child %d process read: %c\n", reader_id, c);
        index++;
        usleep(rand() % 1000000); 
    }
    exit(0);
}


void is_all_ending()
{
    if (status_shared_memory[0] && status_shared_memory[1] && status_shared_memory[2]) // 主进程检测到子进程均已运行结束
    {
        printf("All readers finished reading.\n");
        exit(0);
    }
}


void user_input()
{
    stop_readers();
    char buffer[128];
    printf("Enter additional text: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strlen(buffer)-1]='\0';
    strcat(book_shared_memory, buffer);
    continue_readers();
}

int main()
{
    setbuf(stdout, NULL);
    book_id = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (book_id == -1)
    {
        perror("shmget");
        exit(1);
    }

    book_shared_memory = (char *)shmat(book_id, NULL, 0);
    if (book_shared_memory == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }

    status_id = shmget(IPC_PRIVATE, sizeof(bool) * MAX_READERS, IPC_CREAT | 0666);
    if (status_id == -1)
    {
        perror("shmget");
        exit(1);
    }

    status_shared_memory = (bool *)shmat(status_id, NULL, 0);
    if (status_shared_memory == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }

    memset(book_shared_memory, '\0', SHM_SIZE); // 初始化共享内存为全零字符串
    memset(status_shared_memory, false, sizeof(bool) * MAX_READERS);

    strcpy(book_shared_memory, "BUPT");

    signal(SIGCHLD, is_all_ending);
    for (int i = 0; i < MAX_READERS; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            perror("fork");
            exit(1);
        }
        else if (pids[i] == 0)
        {
            reader_process(i + 1);
        }
    }

    continue_readers(); // 启动所有读进程

    while (1)
    {
        getchar();
        printf("The book now is %s\n", book_shared_memory);
        user_input();
    }
}
