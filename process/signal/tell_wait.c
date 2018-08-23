#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// TELL_WAIT设计思路是，把两个信号SIGUSR1和SIGUSR2来child和parent之间通信的工具。
// 然后，用一个全局标志量sigflag来表示是否已经捕捉了上面两个信号。当然，为了放置在WAIT之前就把这个sigflag就修改了，就需要先把SIGUSR1和SIGUSR2给屏蔽掉，
// 直到开始WAIT，才在sigsuspend函数里面把所有的信号都接受，不屏蔽任何信号。如果进程挂起时，收到了SIGUSR1或SIGUSR2信号，
// 调用了信号处理函数，才会把标志量修改成0，结束sigsuspend的循环。后面就是把信号屏蔽字给修改回去。

static volatile sig_atomic_t sigflag;
static sigset_t newmask, oldmask, zeromask;

// signal handler for SIGUSR1 and SIGUSR2
static void sig_usr(int signo) {
    sigflag = 1;
}

void TELL_WAIT() {
    if (signal(SIGUSR1, sig_usr) == SIG_ERR) {
        fprintf(stderr, "error");
    } 
    if (signal(SIGUSR2, sig_usr) == SIG_ERR) {
        fprintf(stderr, "error");
    }

    sigemptyset(&zeromask);
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigaddset(&newmask, SIGUSR2);

    // 将SIGUSR1和SIGUSR2给阻塞
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
        fprintf(stderr, "error");
    }
}

void TELL_PARENT(pid_t pid) {
    kill(pid, SIGUSR2);
}

void WAIT_PARENT() {
    // 一直挂起，等待parent
    while (sigflag == 0) {
        sigsuspend(&zeromask);
    }
    sigflag = 0;

    // 将屏蔽字给改回去
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
        fprintf(stderr, "error");
    }
}

void TELL_CHILD(pid_t pid) {
    kill(pid, SIGUSR1);
}

void WAIT_CHILD(pid_t pid) {
    while (sigflag == 0) {
        sigsuspend(&zeromask);
    }
    sigflag = 0;

    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
        fprintf(stderr, "error");
    }
}

//不带缓冲的输出
static void charatatime(char *str) {
    char *ptr;
    int c;
    setbuf(stdout, NULL);
    for (ptr = str; (c = *ptr++) != 0; ) {
        putc(c, stdout);
    }
}

int main() {
    pid_t pid;

    // 只使用了三个routine，实现让parent先输出，如果反过来，就需要用另外两个routine
    TELL_WAIT();
    if ((pid = fork()) == 0) {
        WAIT_PARENT();
        charatatime("this is child\n");
        //TELL_PARENT(getppid());
    } else {
        //WAIT_CHILD();
        charatatime("this is parent\n");
        TELL_CHILD(pid);
    }
}