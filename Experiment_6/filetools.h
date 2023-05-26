/*一个二进制文件的读写工具包，hex editor的简单实现。以十六进制的形式读写文件。
本工具包括功能
1.创建新文件：自定义文件名，创建一个空文件
2.写文件：选择以合适的编码方式写入文件
3.读文件：选择以合适的编码方式读取文件
4.修改文件权限
5.退出
主要是使用linux系统调用来实现的。包括open,read,write,close,chmod等等
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// 设置颜色
#define SET_FRONT_COLOR(color) printf("\033[%dm", (color))
// 颜色
#define FRONT_RED 31
#define FRONT_WHITE 37

// 缓冲区大小
#define BUFFER_SIZE 32
#define MAX_FILE_NAME 256   

// 读取文件
void ReadFile(char *file_name);
// 写入文件
void WriteFile(char *file_name, int addr);
// 创建文件
void CreateFile(char *file_name);
// 修改文件模式
void ChangeFileMode(char *file_name);
