# 信号机制

## 信号概念
信号用来提供一种处理异步事件的方法。Linux里面有31中信号，用正整数表示，一般用名字来使用，比如`SIGINT`等。

一个信号出现后，有三种处理方式：
- 忽略信号。大多数信号用这种方式处理，但是`SIGKILL`和`SIGSTOP`信号不能被忽略，因为他们是给内核或者超级用户来终止或者停止进程的方法。
- 捕捉信号。为了能够捕捉信号，要对待捕捉的信号设置一个处理函数，每次接受到信号后，用这个处理函数来执行。
- 执行系统默认动作。大多数信号的系统默认动作是终止该进程。

## signal函数
用来设置捕捉到的信号的处理函数。
```c
typedef void Sigfunc(int);
Sigfunc *signal(int, Sigfunc*);
```
这个函数，接受两个参数，第一个是信号number，第二个是处理函数，返回一个函数指针。

*inx中的`kill`命令并不是真的kill，是用来向某个进程或者进程组来发送信号的，可以用参数来指定信号类型。

子进程会继承父进程的信号处理方式。

## 可重入函数
通常，一个进程捕获到一个信号后，会调用信号处理函数，然后放下本来在做的事情，等信号处理函数执行完后，再继续。但是这样有可能一些信息会被覆盖，那么就会出现位置错误。比如，如果信号处理函数中调用了malloc(),free()，那么就有可能信号处理函数和进程都free()了同一块内存，导致出现SIGSEGV。

所以，在信号处理函数中能够保证调用安全的函数，是可重入函数，也就是 **异步信号安全的**。

一般来说，如果一个函数中使用了:
- 静态数据结构
- malloc(), free()
- 是标准I/O函数
那么，这个函数会是不可重入的。

所以，在一个信号处理函数中使用不可重入函数，会导致不可预知的结果。

## SIGCLD
在早期的Unix里面这个信号存在一些问题，但是在linux里面SIGCLD等同于SIGCHLD。

## pending and blocking
对于每个进程都有一个信号屏蔽字，会把一些信号给堵塞掉。那么，每次待处理的信号就是，在pending并且没有blocking了的信号了。pending列表是用一个bit的list来实现的，对应位置就是对应的信号。另外，如果一个信号在排队的时候，发送了多次信号，还是会只执行一次。

## 发送信号的方式
- kill函数用来将信号发送给进程或者进程组。`int kill(pid_t pid, int signo)`
- raise函数允许进程向自身发送信号。`int raise(int signo)`。`raise(signo)`等于调用`kill(getpid(), signo)`
- alarm函数用来设置一个定时器，会在时间到后，向进程发送信号SIGALARM，默认动作是终止进程
- pause函数会将调用进程挂起，直到捕获到一个信号
  
## 信号集
能够表示多个信号的数据类型是信号集(signal set)，有系列操作函数。

## sigprocmask函数
用来设置进程的信号屏蔽字。函数原型：
```c
int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oset);
```
这个how参数用来指示如何修改当前的信号屏蔽字。

## sigpending函数
返回一个信号集，其中各信号是阻塞不能递送的，所以也一定是未决的。
的的

## sigsuspend函数
这个函数很有意思，它的原型是这样的：
```c
int sigsuspend(const sigset_t *sigmask);
```
它会将进程的信号屏蔽字设置为`sigmask`，然后就开始将进程挂起，在捕捉到一个信号或者发生了一个会终止该进程的信号后，才会返回，并把信号屏蔽字给恢复。

例如下面这段代码就会把`SIGINT`给屏蔽掉，然后挂起进程，直到捕获到一个信号或者发生一个能终止进程的信号。
```c
sigset_t sigmask;
sigemptyset(&sigmask);
sigaddset(&sigmask, SIGINT);

if (sigsuspend(&sigmask) != -1) {
    printf("error");
}
```

回想之前为了解决race condition的一个方法，就是用TELL_WAIT来解决，而这个routine可以用sigsuspend函数来实现。

详细看[这个代码实现](./tell_wait.c)

TELL_WAIT设计思路是，把两个信号SIGUSR1和SIGUSR2来child和parent之间通信的工具。
然后，用一个全局标志量sigflag来表示是否已经捕捉了上面两个信号。当然，为了放置在WAIT之前就把这个sigflag就修改了，就需要先把SIGUSR1和SIGUSR2给屏蔽掉，
直到开始WAIT，才在sigsuspend函数里面把所有的信号都接受，不屏蔽任何信号。如果进程挂起时，收到了SIGUSR1或SIGUSR2信号，
调用了信号处理函数，才会把标志量修改成0，结束sigsuspend的循环。后面就是把信号屏蔽字给修改回去。

