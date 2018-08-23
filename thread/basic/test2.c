#include <pthread.h>
#include <stdio.h>

void cleanup(void *args) {
    printf("cleanup: %s\n", (char*)args);
}

void *init1(void *args) {
    printf("thread1 start\n");
    pthread_cleanup_push(cleanup, (void*)"thread1 first handler");
    pthread_cleanup_push(cleanup, (void*)"thread1 second handler");
    printf("thread1 push complete\n");

    if (args) {
        return ((void*)1);
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    pthread_exit((void*)2);
    return ((void*)1);
}

void *init2(void *args) {
    printf("thread2 start\n");
    pthread_cleanup_push(cleanup, (void*)"thread2 first handler");
    pthread_cleanup_push(cleanup, (void*)"thread2 second handler");
    printf("thread2 push complete\n");

    if (args) {
        return ((void*)2);
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    pthread_exit((void*)2);
}

int main() {
    pthread_t tid1, tid2;
    void *tret;

    pthread_create(&tid1, NULL, init1, (void*)1);
    pthread_create(&tid2, NULL, init2, (void*)2);

    pthread_join(tid1, &tret);
    printf("thread1 exit code %ld\n", (long)tret);

    pthread_join(tid2, &tret);
    printf("thread2 exit code %ld\n", (long)tret);
}