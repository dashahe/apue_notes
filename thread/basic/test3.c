#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t lock;
int a = 0;

void *init(void *vargp) {
    pthread_detach(pthread_self());
    pthread_mutex_lock(&lock);
    a += 1;
    pthread_mutex_unlock(&lock);
    return NULL;
}

void *doit(void *vargp) {
    pthread_t t1;
    pthread_mutex_init(&lock, NULL);
    for (int i = 0; i < 1000; ++i) {
        pthread_create(&t1, NULL, init, NULL);
    }
}

int main() {
    pthread_t root;
    pthread_create(&root, NULL, doit, NULL);
    pthread_join(root, NULL);
    printf("%d", a);
}