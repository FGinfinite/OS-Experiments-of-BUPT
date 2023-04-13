#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


int main()
{
    pid_t pid1, pid2;
    pid1 = fork();
    if (pid1 < 0)
    {
        perror("fork()_error\n");
        return -1;
    }
    else if (pid1 == 0)
    {
        for (int i = 0; i < 9; i++)
        {
            printf("%c", "111111111"[i]);
            fflush(stdout);
        }
    }
    else
    {
        pid2 = fork();
        if (pid2 < 0)
        {
            perror("fork()_error\n");
            return -1;
        }
        else if (pid2 == 0)
        {
            for (int i = 0; i < 10; i++)
            {
                printf("%c", "222222222"[i]);
                fflush(stdout);
            }
        }
        else
        {
            for (int i = 0; i < 10; i++)
            {
                printf("%c", "000000000"[i]);
                fflush(stdout);
            }
        }
    }
    fflush(stdout);
    return 0;
}
