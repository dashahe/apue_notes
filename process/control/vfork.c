#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

char *env_list[] = {
    "USER=unknow",
    "PATH=/tmp",
    NULL
};

int main() {
    pid_t child;

    printf("parent pid: %d\n", getpid());
    if ((child = fork()) == 0) {
        // sleep(1);
        execl("/bin/cat", "/bin/cat", "./vfork.c", (char*)0);
    } else {
        // waitpid(child, NULL, WEXITED);
        exit(0);
    }
}
