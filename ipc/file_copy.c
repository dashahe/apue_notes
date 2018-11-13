#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define PRINT_ERROR(msg) { fprintf(stderr, "%s\n", msg);  _exit(0);  }                         
#define DEF_PAGER "/bin/more"


//读取一个文件，然后通过管道将其传送到子进程，子进程运行了一个
//程序more，专门用来将其分页
int main(int argc, char **argv) {
    int fd[2];
    FILE *fp;
    pid_t pid;
    char buf[1024];
    int n;
    char *pager, *argv0;

    if (argc != 2) {
        PRINT_ERROR("usage: a.out <pathname>");
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        PRINT_ERROR("fopen fail");
    }

    if (pipe(fd) < 0) {
        PRINT_ERROR("pipe error");
    }
    
    if ((pid = fork()) < 0) {
        PRINT_ERROR("fork fail");
    } else if (pid > 0) {   //parent
        close(fd[0]);
        while (fgets(buf, 1024, fp) != NULL) {
            n = strlen(buf);
            if (write(fd[1], buf, n) != n) {
                PRINT_ERROR("write error to pipe");
            }
            // puts(buf);
        }
        close(fd[1]);
        waitpid(pid, NULL, 0);
    } else {                //child
        //子进程从fd[0]读取数据，重定向到标准输入
        close(fd[1]);

        if (fd[0] != STDIN_FILENO) {
            if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO) {
                PRINT_ERROR("dup2 error");
            }
            close(fd[0]);
        }

        if ((pager = getenv("PAGER")) == NULL) {
            pager = DEF_PAGER;
        }
        if ((argv0 = strrchr(pager, '/')) != NULL) {
            argv0++;
        } else {
            argv0 = pager;
        }

        puts(pager);
        puts(argv0);
        // if (execl(pager, argv0, (char*)0) < 0) {
        //     PRINT_ERROR("execl error");
        // }
        execv(pager, argv);

        _exit(0);
    }

    return 0;
}
