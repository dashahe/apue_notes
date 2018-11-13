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

static char client_fifo[CLIENT_FIFO_NAME_LEN];
static void remove_fifo() {
    unlink(client_fifo);
}

int main(int argc, char const *argv[])
{
    int server_fd, client_fd, dummy_fd;
    struct request req;
    struct responce resp;

    umask(0);

    snprintf(client_fifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long)getpid());
    if (mkfifo(client_fifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1) {
        PRINT_ERROR("mkfifo fail");
    }

    if (atexit(remove_fifo) != 0) {
        PRINT_ERROR("register exit function fail");
    }


    server_fd = open(SERVER_FIFO, O_WRONLY);
    if (server_fd == -1) {
        PRINT_ERROR("open server fifo fail");
    }

    req.pid = getpid();
    req.seq_len = rand() % 10;
    if (write(server_fd, &req, sizeof(struct request)) != sizeof(struct request)) {
        PRINT_ERROR("write error");
    }

    client_fd = open(client_fifo, O_RDONLY);
    if (client_fd == -1) {
        PRINT_ERROR("open client fifo fail");
    }
    if (read(client_fd, &resp, sizeof(resp)) != sizeof(resp)) {
        PRINT_ERROR("read responce fail");
    }

    printf("%d\n", resp.seq_num);

    exit(EXIT_SUCCESS);

}
