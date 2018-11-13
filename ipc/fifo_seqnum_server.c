#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define PRINT_ERROR(msg) { fprintf(stderr, "%s\n", msg);  _exit(0);  }                         

#define SERVER_FIFO "/tmp/seq_server"
#define CLIENT_FIFO_TEMPLATE "/tmp/seq_client.%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)

struct request {
    pid_t pid;
    int seq_len;
};

struct responce {
    int seq_num;
};

int main(int argc, char const *argv[]) {

    int server_fd, client_fd, dummy_fd;
    char client_fifo[CLIENT_FIFO_NAME_LEN];
    struct request req;
    struct responce resp;
    int seq_num = 0;

    umask(0);

    if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1) {
        PRINT_ERROR("make server fifo fail");
    }

    server_fd = open(SERVER_FIFO, O_RDONLY);
    if (server_fd == -1) {
        PRINT_ERROR("open server fifo fail, server");
    }

    //以只读的方式打开server fifo，防止暂时没有client打开server fifo时，
    //server_fd read的时候返回EOF
    dummy_fd = open(SERVER_FIFO, O_WRONLY);
    if (dummy_fd == -1) {
        PRINT_ERROR("open server fifo fail, dummy");
    }

    //将SIGPIPE信号处理函数设置为忽略，防止尝试向一个没有以读方式打开的fifo写入数据时
    //会收到SIGPIPE信号杀死进程
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        PRINT_ERROR("signal error");
    }

    for (;;) {
        if (read(server_fd, &req, sizeof(req)) != sizeof(req)) {
            fprintf(stderr, "error in reading request.");
            continue;
        }

        //生成客户端FIFO的path
        snprintf(client_fifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, req.pid);

        client_fd = open(client_fifo, O_WRONLY);
        if (client_fd == -1) {
            fprintf(stderr, "open %s error.", client_fifo);
            continue;
        }

        resp.seq_num = seq_num;
        if (write(client_fd, &resp, sizeof(resp)) != sizeof(struct responce)) {
            PRINT_ERROR("write error to client");
        }
        if (close(client_fd) == -1) {
            PRINT_ERROR("close error");
        }

        printf("receive data from:%d,(%d) responce:%d\n", req.pid, req.seq_len, resp.seq_num);
        seq_num += req.seq_len;
    }

    return 0;
}
