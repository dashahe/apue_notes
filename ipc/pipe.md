## 进程间通信
![Alt text](./1542002231318.png)

IPC主要分为三类：通信、同步和信号。

### 管道
最古老的通信机制，两点局限性：
- 历史上是半双工的，虽然现在某些系统支持全双工管道。
- 只能在具有公共祖先的两个进程间使用。

特征：
- 一个管道是一个字节流，不存在消息边界的概念
- 从管道中读取数据，如果为空，那么会阻塞到有数据
- 管道是单向的
- 写入不超过PIPE_BUF字节的操作是原子的
- 管道的容量是有限的

管道创建函数：
```c
#include <unistd.h>
int pipe(int fd[2]);
```

调用该函数会创建一个管道，并将其两端作为返回值放在fd数组中。其中`fd[1] -> fd[0]`，也就是说`fd[1]`写数据，`fd[0]`读数据。

![Alt text](./1541990555254.png)

然后`fork`一下，就可以在父子进程之间通信了：
![Alt text](./1541990627697.png)
![Alt text](./1541990634344.png)

因为`fork`会继承父进程的文件描述符表，所以我们就可以实现通信，然后关闭不需要的文件描述符就好。如果另外一端关闭，会这样：
- read一个写端已经关闭的管道，在所有数据读完后，read返回0。
- write一个读端已经关闭的管道，产生信号SIGPIPE，write返回-1，errno设置为EPIPE。

#### 基本使用
使用例子：
```c 
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
```

管道主要的用途：实现一种进程间同步方式（TELL_WAIT机制）、连接过滤器（比如拼接ls和wc）。

#### 管道实现ls | wc
具体是`fork`出两个子进程，然后分别运行`ls`和`wc`。`ls`那个子进程将`fd[0]`关掉，将其标准输出重定向到`fd[1]`。`wc`那个子进程将`fd[1]`关掉，将其标准输入重定向到`fd[0]`，这样`ls`子进程的输出就变成了`wc`的输入。
code: [ls_and_wc.c](./ls_and_wc.c)

#### 进程间同步 TELL_WAIT的实现
code: [tell_wait.c](./tell_wait.c)

### popen函数和pclose函数
管道常见操作就是创建一个连接到另一个进程的管道，然后读其输出或者向其输入写数据，所以标准I/O库提供了`popen`和`pclose`函数来简化管道。

具体是，创建一个管道，fork一个子进程，关闭没有使用的管道端，执行shell命令，然后等待命令终止。

```c 
#include <stdio.h>
FILE *popen(const char *cmdstring, const char *type);
int pclose(FILE *fp);
```

类似于`open()`，`popen`的第二个参数为"r"或者"w"。如果是"r"，就关闭子进程的管道输入端等。cmdstring是将要以`sh -c cmdstring`的命令。

使用`popen`比较需要注意的是，采用"w"模式的时候，返回的文件指针所对应的缓冲区是**全缓冲**的，因为这个文件指针不是与IO设备相关的。只有与IO设备相关，缓冲才默认是行缓冲的。

所以，有可能我们向fp写入数据，但是没有写满缓冲区，那么可能子进程就会阻塞在`read`，而父进程写数据完了，阻塞在`read`子进程数据，那么就会造成死锁。解决方法是定期`fflush()`，或者采用`setbuf(fp, NULL)`的方式来将其设置为无缓冲。

### FIFO 命名管道
key: 相比管道优点、文件类型的一种、使用方法、非阻塞、write && read异常情况。

管道的两个局限性（具有相同祖先进程的进程间才能使用和半双工）的第一个可以用FIFO来解决，因为它有名字，所以可以在两个独立的进程间使用。

其他方面，FIFO和管道类似，比如都是写入具有原子性，前提是写入的数据大小不超过`PIPE_BUF`。

FIFO也是一种文件类型，所以操作也是`mkfifo`、`open`、`read`、`write`、`close`之类。

```c
#include <sys/stat.h>
int mkfifo(const char *pathname, mode_t mode); 
```
这个函数用来创建FIFO，两个参数分别是路径名和mode，mode指定新FIFO的权限，是一系列常数的or，然后根据umask的值来取mask。

还有呢，就是
- 如果FIFO的最后一个写进程关闭了，那么`read`一个没有以只写方式打开的管道，会返回一个EOF。
- 如果`write`一个没有以只读方式打开的管道，会产生`SIG_PIPE`信号，该信号默认处理方式会杀死该进程。

另外，FIFO也支持非阻塞I/O。默认情况下，只读open会阻塞到其他进程为写而打开该FIFO，只读open会阻塞到某个进程为读而打开该FIFO为止。

如果指定了`O_NONBLOCK`，只读open立即返回，如果失败会返回-1，如果成功返回分配的`fd`。
```c
fd = open("fifopath", O_RDONLY | O_NONBLOCK);
if (fd == -1) {
	//error
}
```

#### FIFO example
先看一下FIFO和tee结合创建双重管道：

![Alt text](./1542078503873.png)
![Alt text](./1542078645075.png)

tee会将其标准输入同时发送到标准输出和其参数指定的地方，这里特定的是myfifo，而myfifo指定的输出是`wc -l`的标准输出，所以会同时输出12和sort后的内容。

然后我们采用FIFO来实现一下server/client模型：
![Alt text](./1542078690177.png)

该模型是，若干client通过server_fifo向server发送request，其中包括pid和seq_len。server通过server_fifo收到消息后，对每个不同的client，通过与其pid对应的client_fifo回复responce，其中包括该request对应的seq_num。

这里要注意的是，服务端`server_fd = open(SERVER_FIFO, O_RDONLY);`后需要`dummy_fd = open(SERVER_FIFO, O_WRONLY);`。因为，可能server_fd以只读方式打开后，然后去`read`，但是这个时候没有`client`以只写方式打开server_fifo，那么server的read就会返回一个`EOF`。所以，需要在server这边假装打开一下，防止返回`EOF`。

具体代码：[fifo_seqnum_server.c](./fifo_seqnum_server.c), [fifo_seqnum_client.c](./fifo_seqnum_client.c)。

运行结果：![Alt text](./1542079209969.png)

总结一下，FIFO的使用和文件类似:

- 需要先`mkfifo`来创建FIFO，其中指定了pathname和mode。
- `open(SERVER_FIFO, O_WRONLY)`这种，传入FIFO路径和打开方式。
- 然后就是read或者write，比如`read(client_fd, &buf, len)`。
- 最后需要`close(fd)`。

### 消息对列