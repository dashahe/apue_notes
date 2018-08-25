#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

//三个处理函数

//lock all mutex
void prepare() {
    printf("prepare called...\n");
    pthread_mutex_lock(&lock1);
    pthread_mutex_lock(&lock2);
}

void parent() {
    printf("parent called...\n");
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);
}

void child() {
    printf("child called...\n");
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);

}

void *thread_run(void *arg) {
    printf("thread create...\n");
    pause();
    return 0;
}

int main() {
    pid_t pid;
    pthread_t tid;

    //设置fork的锁的处理函数
    pthread_atfork(prepare, parent, child);
    pthread_create(&tid, NULL, thread_run, NULL);

    sleep(2);
    printf("parent is going to fork...\n");

    if ((pid = fork()) == 0) {
        printf("child return...\n");
    } else {
        printf("parent return...\n");
    }
    exit(0);
}