#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *init(void *args) {
    printf("new thread created: %lu\n", pthread_self());
    return ((void*)0);
}

int main() {
    pthread_t tid;
    int err = pthread_create(&tid, NULL, init, NULL);
    if (err != 0) {
        fprintf(stderr, "create fail!");
    }
    sleep(1);
}