#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

void *server_fn(void *arg) {
    system("./server");
    pthread_exit(NULL);
}

void *client_fn(void *arg) {
    system("./client");
    pthread_exit(NULL);
}

int main() {
    pthread_t server_thread, client_thread;

    // 创建server线程
    pthread_create(&server_thread, NULL, server_fn, NULL);

    // 创建client线程
    pthread_create(&client_thread, NULL, client_fn, NULL);

    // 等待线程结束
    pthread_join(server_thread, NULL);
    pthread_join(client_thread, NULL);

    return 0;
}

