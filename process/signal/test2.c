#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static void alarm_handler(int signo) {
    printf("process: %d wake up~\n", getpid());
}

// 传入的参数不能为0，因为设置了0秒后唤醒，然而这个时候pause()函数还没有执行，就可嫩永远睡着了
unsigned int my_sleep(unsigned int seconds) {
    if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
        return (seconds);
    }
    alarm(seconds); //设置在seconds后唤醒继承
    pause(); //进程去睡觉
    return (alarm(0));
}

int main() {
    for (int i = 1; i < 10; ++i) {
        my_sleep(i);
    }
}