#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *init(void *arg) {
    sleep(0.1);
    fprintf(stdout, "hello world");
}

int main() {
    pthread_t t;
    pthread_attr_t my_attr;

    //初始化线程属性，并设置为detach状态
    pthread_attr_init(&my_attr);
    pthread_attr_setdetachstate(&my_attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&t, &my_attr, init, NULL);
    // pthread_join(t, NULL);

    pthread_attr_destroy(&my_attr);
}