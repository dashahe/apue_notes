#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid;

    if (pipe(fd) < 0) {
        fprintf(stderr, "pipe error");
        _exit(0);
    }

    if ((pid = fork()) < 0) {
        fprintf(stderr, "pipe error");
        _exit(0);
    } else if (pid > 0) { //parent
        close(fd[0]);
        char *s = "this is a test message from parent process.";
        write(fd[1], s, strlen(s));
        waitpid(pid, NULL, 0);
    } else {               //child
        close(fd[1]);
        char buf[1024];
        int n = read(fd[0], buf, n);
        puts(buf);
    }
    _exit(0);
}