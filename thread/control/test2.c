#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex;
pthread_mutexattr_t attr;

int main() {
    pthread_t t;

    pthread_mutexattr_init(&attr);
     //设置为递归型互斥锁
    // pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT);

    pthread_mutex_init(&mutex, &attr);

    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&mutex);

    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&mutex);
}