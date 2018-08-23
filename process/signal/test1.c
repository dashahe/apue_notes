#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static void fun(int signo) {
    if (signo == SIGINT) {
        printf("Aha, not interrupt!\n");
    } else if (signo == SIGTSTP) {
        printf("Aha, not stop!\n");
    } else {
        printf("not set!\n");
    }
}

char a[10] = "hello";

int main() {
    if (signal(SIGINT, fun) == SIG_ERR) {
        printf("can't catch SIGINT\n");
    } 
    if (signal(SIGTSTP, fun) == SIG_ERR) {
        printf("can't catch SIGTSTP\n");
    }
    if (fork() == 0) {
        a[0] = 'b';
        printf("child: %p %s\n", a, a);
    } else {
        printf("parent: %p %s\n", a, a);
    }
    while (1) {
        pause();
    }
}