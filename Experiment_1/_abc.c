/*2号实验的目的是希望能看到一串混合输出的字符串，使实验者更清楚地看到进程的异步执行。这里则改为了反复输出'a''b''c'三个字符，运行后也是一样的效果，同样会混合输出。同时也相当于做了1号实验，所以此处就不贴出冗余的代码了。*/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        for (size_t i = 0; i < 10; ++i)
        {
            printf("b");
            // printf("I am child process 1\n");
            fflush(stdout);
            sleep(1);
        }
        exit(0);
    }
    else
    {
        pid_t pid2 = fork();
        if (pid2 == 0)
        {
            for (size_t i = 0; i < 10; ++i)
            {
                printf("c");
                // printf("I am child process 2\n");
                fflush(stdout);
                sleep(1);
            }
            exit(0);
        }
    }
    for (size_t i = 0; i < 10; ++i)
    {
        printf("a");
        fflush(stdout);
        sleep(1);
    }
    return 0;
}