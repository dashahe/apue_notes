#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define PRINT_ERROR(msg) { fprintf(stderr, "%s", msg);  _exit(0);  }    

int main(int argc, char const *argv[])
{
    int pfd[2];

    if (pipe(pfd) == -1) {
        PRINT_ERROR("pipe error");
    }

    switch(fork()) {
        case -1:
            PRINT_ERROR("fork error");
        case 0: //child
            if (close(pfd[0]) == -1) {
                PRINT_ERROR("close pfd[0]");
            }

            //将子进程的输出端重定向到管道pfd[1]
            if (pfd[1] != STDOUT_FILENO) {
                if (dup2(pfd[1], STDOUT_FILENO) == -1) {
                    PRINT_ERROR("dup2 error");
                }
                if (close(pfd[1]) == -1) {
                    PRINT_ERROR("close error");
                }
            }
            execlp("ls", "ls", (char*)NULL);
            PRINT_ERROR("execlp ls error");


        default: //parent
            break;
    }

    switch(fork()) {
        case -1:
            PRINT_ERROR("fork error");
        case 0: //child
            if (close(pfd[1]) == -1) {
                PRINT_ERROR("close pfd[0]");
            }

            //将子进程的输出端重定向到管道pfd[1]
            if (pfd[0] != STDIN_FILENO) {
                if (dup2(pfd[0], STDIN_FILENO) == -1) {
                    PRINT_ERROR("dup2 error");
                }
                if (close(pfd[0]) == -1) {
                    PRINT_ERROR("close error");
                }
            }
            execlp("wc", "wc", (char*)NULL);
            PRINT_ERROR("execlp ls error");


        default: //parent
            break;
    }
    return 0;
}
