#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>

int become_daemon() {
    int maxfd, fd;

//step1, fork and setsid and fork again
    switch (fork()) {
        case -1: return -1;
        case 0: break;     //children
        default: _exit(0); //parent
    }

    if (setsid() == -1) {
        return -1;
    }

    switch(fork()) {
        case -1: return -1;
        case 0: break;
        default: _exit(0);
    }

//step2, clear umask
    umask(0);

//step3, change current work directory to root
    chdir("/");

//step4, close fd and dup STDIN, STDOUT, STDERR to /dev/null
    maxfd = sysconf(_SC_OPEN_MAX);
    for (fd = 0; fd < maxfd; ++fd) {
        close(fd);
    }
    close(STDIN_FILENO);
    fd = open("/dev/null", O_RDWR);

    if (fd != STDIN_FILENO) {
        return -1;   
    }
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
        return -1;
    }
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
        return -1;
    }

    return 0;
}

int main() {
    int x = become_daemon();
    syslog(LOG_WARNING, "return val: %d and my pid is  %d", x, getpid());
    closelog();
    sleep(100);
}