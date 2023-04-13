#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
int main()
{

    int status_id;
    bool *status;
    status_id = shmget(IPC_PRIVATE, sizeof(bool) * 3, IPC_CREAT | 0666);
    status = (bool *)shmat(status_id, NULL, 0);
    memset(status, false, sizeof(bool) * 3);
    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        for (size_t i = 0; i < 10; ++i)
        {
            while (status[1] != true)
                ;
            printf("b");
            fflush(stdout);
            status[1] = false;
            status[2] = true;
        }
    }
    else
    {
        pid_t pid2 = fork();
        if (pid2 == 0)
        {
            for (size_t i = 0; i < 10; ++i)
            {
                while (status[2] != true)
                    ;
                printf("c\n");
                fflush(stdout);
                status[2] = false;
                status[0] = true;
            }
        }
        else
        {
            status[0] = true;
            for (size_t i = 0; i < 10; ++i)
            {
                while (status[0] != true)
                    ;
                printf("%d:a", (int)i);
                fflush(stdout);
                status[0] = false;
                status[1] = true;
            }
        }
    }

    return 0;
}
