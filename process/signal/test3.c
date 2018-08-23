#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);

    if (sigsuspend(&sigmask) != -1) {
        printf("error");
    }
}