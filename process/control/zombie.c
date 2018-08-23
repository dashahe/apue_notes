#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    pid_t pid;
    if ((pid = fork()) == 0) {
        exit(0);
    } else {
        sleep(1);
        char cmd[100];
        sprintf(cmd, "ps -aux | grep %d", pid);
        printf("%s\n", cmd);
        system(cmd);
        exit(0);
    }
}