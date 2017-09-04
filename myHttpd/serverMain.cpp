#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "threadpool.h"
#include "myHttpd.h"

#define MAX_FD 65536
#define MAX_EVENT_NUMBER 10000

const char *config_path = "./myHttpd.config";

extern int addfd(int epollfd, int fd, bool one_shot);
extern int removefd(int epollfd, int fd);

int http_conn::m_epollfd = -1;
int http_conn::m_user_count = 0;

void addsig(int sig, void(handler)(int), bool restart = true) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;

    //当信号处理函数返回时，被信号中断的系统调用将自动恢复
    if (restart) {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void show_error(int connfd, const char *info) {
    fprintf(stderr,"%s", info);
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: <Port>\n");
    }

    const char *ip  = argv[1];
    int port = atoi(argv[2]);

    //忽略SIGPIPE信号
    addsig(SIGPIPE, SIG_IGN);
    printf("abc");

    threadpool<http_conn> *pool = NULL;
    try {
        pool = new threadpool<http_conn>;
    }
    catch(...) {
        return 1;
    }

    //读取config内容

    //预先分配好http_conn对象
    http_conn *users = new http_conn[MAX_FD];
    assert(users);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    //强制断开，不会发送未发送完成的数据
    struct linger temp = {1,0};
    setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &temp, sizeof(temp));

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);

    ret = listen(listenfd, 5);
    assert(ret >= 0);


    struct epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);

    addfd(epollfd, listenfd, false);
    http_conn::m_epollfd = epollfd;

    while (1) {
        int nfds = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((nfds < 0) && errno != EINTR) {
            fprintf(stderr, "epoll failure\n");
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd) {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
                if (connfd < 0) {
                    fprintf(stderr, "Errno: %d\n", errno);
                    continue;
                }

                if (http_conn::m_user_count >= MAX_FD) {
                    show_error(connfd, "Internal server too busy\n");
                    continue;
                }

                //初始化当前客户连接
                users[connfd].init(connfd, client_address);
                printf("init\n");
            }
            //连接发生异常, 直接关闭客户连接
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                users[sockfd].close_conn();
            }

            //有数据可读
            else if (events[i].events & EPOLLIN) {
                printf("have data\n");
                //根据read的结果，选择进一步处理还是丢弃
                if (users[sockfd].read())
                    pool->append(users+sockfd);
                else
                    users[sockfd].close_conn();
            }

            //有数据可写
            else if (events[i].events & EPOLLOUT) {
                if (!users[sockfd].write())
                    users[sockfd].close_conn();
            }
            else {
                users[sockfd].close_conn();
            }
        }
    }

    close (epollfd);
    close (listenfd);
    delete []users;
    delete pool;
    return 0;
}

