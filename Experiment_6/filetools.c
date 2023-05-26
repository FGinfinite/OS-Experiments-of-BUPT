#include "filetools.h"

void ReadFile(char *file_name)
{
    // 以十六进制与对应UTF-8字符的方式输出文件所有内容
    int f = open(file_name, O_RDONLY);
    if (f == -1)
    {
        perror("Error opening file");
        return;
    }

    struct stat statbuf;
    stat(file_name, &statbuf);
    // 获取文件大小对应的十六进制格式的长度
    int file_size = statbuf.st_size;
    int hex_len = 0;
    for (int i = file_size; i > 0; i /= 16)
    {
        hex_len++;
    }
    // 生成地址格式
    char addr_format[BUFFER_SIZE] = {0};
    sprintf(addr_format, "0x%%0%dx: ", hex_len);

    char buffer[BUFFER_SIZE] = {0};
    ssize_t read_size = 0;
    for (int j = 0; (read_size = read(f, buffer, sizeof(buffer))) != 0; ++j)
    {
        printf(addr_format, j * BUFFER_SIZE);
        for (int i = 0; i < read_size; i++)
        {
            printf("%02X ", buffer[i]);
        }
        printf("\n");
        // 输出对应长度的空格以对齐
        printf("%*s", hex_len + 4, "");
        for (int i = 0; i < read_size; i++)
        {
            // 计算buffer[i]对应的UTF-8字符长度
            // 如果是可打印字符，输出对应字符，否则输出空格
            if (buffer[i] >= 32 && buffer[i] <= 126)
            {
                printf(" %c ", buffer[i]);
            }
            else
            {
                printf("   ");
            }
        }
        printf("\n");
    }
    return;
}

void WriteFile(char *file_name, int addr)
{
    // 打开文件并获取文件信息
    int f = open(file_name, O_RDWR);
    if (f == -1)
    {
        perror("Error opening file");
        return;
    }
    struct stat statbuf;
    stat(file_name, &statbuf);

    // 定位文件指针到addr对应的行
    if (addr == -1)
    {
        addr = statbuf.st_size; // 如果addr为-1，定位到文件末尾
    }

    int line_addr = addr - addr % BUFFER_SIZE; // 行开头地址
    int line = addr / BUFFER_SIZE;             // 行号
    lseek(f, line_addr, SEEK_SET);

    // 打印地址信息
    char addr_format[BUFFER_SIZE] = {0};
    int hex_len = 0;
    for (int i = statbuf.st_size; i > 0; i /= 16)
    {
        hex_len++;
    }
    sprintf(addr_format, "0x%%0%dx: ", hex_len); // 0x%x: 格式
    printf(addr_format, line_addr);

    // 打印整行内容
    char buffer[BUFFER_SIZE] = {0};
    ssize_t read_size = read(f, buffer, sizeof(buffer));
    for (int i = 0; i < read_size; i++)
    {
        if (i == addr % BUFFER_SIZE)
        {
            SET_FRONT_COLOR(FRONT_RED);
            printf("%02X ", buffer[i]);
            SET_FRONT_COLOR(FRONT_WHITE);
        }
        else
        {
            printf("%02X ", buffer[i]);
        }
    }
    if (addr == statbuf.st_size)
    {
        SET_FRONT_COLOR(FRONT_RED);
        printf("□□ \n");
        SET_FRONT_COLOR(FRONT_WHITE);
    }
    else
        printf("\n");
    // 输出对应长度的空格以对齐
    printf("%*s", hex_len + 4, "");
    for (int i = 0; i < read_size; i++)
    {
        if (i == addr % BUFFER_SIZE)
        {
            SET_FRONT_COLOR(FRONT_RED);
            if (buffer[i] >= 32 && buffer[i] <= 126)
            {
                printf(" %c ", buffer[i]);
            }
            else
            {
                printf(" □ ");
            }
            SET_FRONT_COLOR(FRONT_WHITE);
        }
        else
        {
            if (buffer[i] >= 32 && buffer[i] <= 126)
            {
                printf(" %c ", buffer[i]);
            }
            else
            {
                printf("   ");
            }
        }
    }
    if (addr == statbuf.st_size)
    {
        SET_FRONT_COLOR(FRONT_RED);
        printf("□ \n");
        SET_FRONT_COLOR(FRONT_WHITE);
    }
    else
        printf("\n");

    // 修改文件内容
    printf("Please input the new content: ");
    char intput_buffer[BUFFER_SIZE] = {0};
    scanf(" %[^\n]", intput_buffer);
    lseek(f, addr, SEEK_SET);
    write(f, intput_buffer, strlen(intput_buffer));
    printf("Write successfully!\n");

    // 输出改后的行
    printf(addr_format, line_addr);
    lseek(f, line_addr, SEEK_SET);
    read_size = read(f, buffer, sizeof(buffer));
    for (int i = 0; i < read_size; i++)
    {
        if (i == addr % BUFFER_SIZE)
        {
            SET_FRONT_COLOR(FRONT_RED);
            printf("%X ", buffer[i]);
            SET_FRONT_COLOR(FRONT_WHITE);
        }
        else
        {
            printf("%X ", buffer[i]);
        }
    }
    printf("\n");
    // 输出对应长度的空格以对齐
    printf("%*s", hex_len + 4, "");
    for (int i = 0; i < read_size; i++)
    {
        if (i == addr % BUFFER_SIZE)
        {
            SET_FRONT_COLOR(FRONT_RED);
            if (buffer[i] >= 32 && buffer[i] <= 126)
            {
                printf(" %c ", buffer[i]);
            }
            else
            {
                printf(" □ ");
            }
            SET_FRONT_COLOR(FRONT_WHITE);
        }
        else
        {
            if (buffer[i] >= 32 && buffer[i] <= 126)
            {
                printf(" %c ", buffer[i]);
            }
            else
            {
                printf("   ");
            }
        }
    }
    printf("\n");
    return;
}

void CreateFile(char *file_name)
{
    int f = open(file_name, O_RDWR | O_CREAT, 0666);
    if (f == -1)
    {
        perror("Error opening file");
        return;
    }
    printf("Create file successfully!\n");
    close(f);
    return;
}

void ChangeFileMode(char *file_name)
{
    // 若文件存在则输出其文件信息
    struct stat statbuf;
    if (stat(file_name, &statbuf) == 0)
    {
        printf("File id: %lu\n", statbuf.st_ino);
        printf("File group id: %d\n", statbuf.st_gid);
        printf("File size: %ld\n", statbuf.st_size);
        printf("File type: ");
        {
            if (S_ISREG(statbuf.st_mode))
                printf("Regular file\n");
            else if (S_ISDIR(statbuf.st_mode))
                printf("Directory\n");
            else if (S_ISCHR(statbuf.st_mode))
                printf("Character device\n");
            else if (S_ISBLK(statbuf.st_mode))
                printf("Block device\n");
            else if (S_ISFIFO(statbuf.st_mode))
                printf("FIFO\n");
            else if (S_ISLNK(statbuf.st_mode))
                printf("Symbolic link\n");
            else if (S_ISSOCK(statbuf.st_mode))
                printf("Socket\n");
        }
        printf("File permission: ");
        {
            if (statbuf.st_mode & S_IRUSR)
                printf("r");
            else
                printf("-");
            if (statbuf.st_mode & S_IWUSR)
                printf("w");
            else
                printf("-");
            if (statbuf.st_mode & S_IXUSR)
                printf("x");
            else
                printf("-");
            if (statbuf.st_mode & S_IRGRP)
                printf("r");
            else
                printf("-");
            if (statbuf.st_mode & S_IWGRP)
                printf("w");
            else
                printf("-");
            if (statbuf.st_mode & S_IXGRP)
                printf("x");
            else
                printf("-");
            if (statbuf.st_mode & S_IROTH)
                printf("r");
            else
                printf("-");
            if (statbuf.st_mode & S_IWOTH)
                printf("w");
            else
                printf("-");
            if (statbuf.st_mode & S_IXOTH)
                printf("x");
            else
                printf("-");
            printf("\n");
        }
    }
    else
    {
        printf("The file does not exist!\n");
        return;
    }
    // 修改文件权限
    printf("Please input the new permission: ");
    char permission[10] = {0};
    scanf("%s", permission);
    mode_t new_permission = 0;
    // 依次处理每个字符
    for (int i = 0; i < 9; i++)
    {
        new_permission <<= 1; // 将模式左移一位

        if (permission[i] != '-')
        {
            new_permission |= 1; // 如果字符不是 '-'，则将对应位设置为 1
        }
    }
    if (chmod(file_name, new_permission) == 0)
    {
        printf("Modify permission successfully!\n");
    }
    else
    {
        printf("Modify permission failed!\n");
    }
}

int main()
{
    int choice = 0;
    char file_name[MAX_FILE_NAME] = {0};
    int addr = 0;
    while (1)
    {
        printf("Please input the operation you want to do:\n");
        printf("1. Create a file\n");
        printf("2. Read a file\n");
        printf("3. Write a file\n");
        printf("4. Modify file permissions\n");
        printf("5. Exit\n");
        printf("Your choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            printf("Please input the file name: ");
            scanf(" %[^\n]", file_name);
            CreateFile(file_name);
            break;
        case 2:
            printf("Please input the file name: ");
            scanf(" %[^\n]", file_name);
            ReadFile(file_name);
            break;
        case 3:
            printf("Please input the file name: ");
            scanf(" %[^\n]", file_name);
            printf("Please input the address (decimal; '-1'stands for end of file): ");
            scanf("%d", &addr);
            WriteFile(file_name, addr);
            break;
        case 4:
            printf("Please input the file name: ");
            scanf(" %[^\n]", file_name);
            ChangeFileMode(file_name);
            break;
        case 5:
            return 0;
        default:
            printf("Invalid input!\n");
            break;
        }
    }
    return 0;
}