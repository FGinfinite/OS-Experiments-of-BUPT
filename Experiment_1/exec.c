/*本程序因为内容较为简单，经过集体讨论后小组成员所编写的代码都十分差不多。所以本组在原本简陋的程序中添加了一些额外的内容后决定将本程序作为实验结果。*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    int status;
    pid = fork(); // 创建子进程

    if (pid == -1) { 
        // fork()调用失败
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { 
        // 子进程
        char *args[] = {"./test/test", NULL}; // 替换成要执行的程序及参数
        execvp(args[0], args); // 替换子进程内容
        perror("exec");
        exit(EXIT_FAILURE);
    } else { 
        // 父进程
        printf("I am the parent process, waiting for child...\n");
        wait(&status); // 等待子进程结束
        printf("Child exited with status %d.\n", status);
    }

    return 0;
}
