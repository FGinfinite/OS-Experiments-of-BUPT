#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>


int main()
{
    pid_t pid = fork();
    if (pid == 0)
    {
        execl("./sever", "NULL", NULL);
    }
    else if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid == 0)
    {
        execl("./client", "NULL", NULL);
    }
    else if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    wait(NULL);
    wait(NULL);
}