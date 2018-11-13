#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define PRINT_ERROR(msg) { fprintf(stderr, "%s", msg);  _exit(0);  }                         


//pfd1用于父进程向子进程通知
//pfd2用于子进程向父进程通知
static int pfd1[2], pfd2[2];

void TELL_WAIT() {
    if (pipe(pfd1) < 0 || pipe(pfd2) < 0) {
        PRINT_ERROR("pipe error");
    }
}

void TELL_CHILD(pid_t pid) {
    if (write(pfd1[1], "p", 1) != 1) {
        PRINT_ERROR("TELL CHILD ERROR");
    }
}

void TELL_PARENT(pid_t pid) {
    if (write(pfd2[1], "c", 1) != 1) {
        PRINT_ERROR("TELL PARENT ERROR");
    }
}

void WAIT_CHILD() {
    char c;
    if (read(pfd2[0], &c, 1) != 1) {
        PRINT_ERROR("read error");
    }
    if (c != 'c') {
        PRINT_ERROR("WAIT CHILD: wrong data");
    }
}

void WAIT_PARENT() {
    char c;
    if (read(pfd1[0], &c, 1) != 1) {
        PRINT_ERROR("read error");
    }
    if (c == 'p') {
        PRINT_ERROR("WAIT PARENT: wrong data");
    }
}


int main(int argc, char **argv) {
    pid_t pid;

    setbuf(stdout, NULL);

    TELL_WAIT();
    switch((pid = fork())) {
        case -1:
            PRINT_ERROR("fork error");
        case 0:
            for (int i = 0; i < 100000; ++i) {
                printf("%d: child hello world how are you\n", i);
            }
            TELL_PARENT(pid);
            break;
        default:
            WAIT_CHILD();
            for (int i = 0; i < 100000; ++i) {
                printf("%d: parent hello world how are you \n", i);
            }
    }
    return 0;
}
 