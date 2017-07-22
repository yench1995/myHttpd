#include "myHttpd.h"

#define ONEKILO    1024
#define ONEMEGA    1024 * ONEKILO
#define ONEGIGA    1024 * ONEMEGA

void *thread_func(void *param);
int thread_num = 0;
pthread_mutex_t thread_num_mutex = PTHREAD_MUTEX_INITIALIZER;

void thread_num_add1()
{
    pthread_mutex_lock(&thread_num_mutex);
    ++thread_num;
    pthread_mutex_unlock(&thread_num_mutex);
}

void thread_num_minus1()
{
    pthread_mutex_lock(&thread_num_mutex);
    --thread_num;
    pthread_mutex_unlock(&thread_num_mutex);
}

void *thread_func(void *param)
{
    thread_num_add1();
    http_header_t *phttphdr = alloc_http_header();

    epollfd_connfd *ptr_epollfd_connfd = (epollfd_connfd*)param;
    int conn_sock = ptr_epollfd_connfd->connfd;

    struct epoll_event ev, events[2];
    ev.events = EPOLLIN|EPOLLET;
    ev.data.fd = conn_sock;

}

int main(int argc, char **argv)
{
    int listenfd, connfd, epollfd;
    int nfds;
    int listen_port;

    struct servent *pservent;
    struct epoll_event ev, events[MAX_EVENTS];

    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;

    socklen_t addrlen;
    pthread_attr_t pthread_attr_detach;
    epollfd_connfd epollfd_connfd;
    pthread_t tid;

    if(argc != 2)
    {
        printf("Usage: %s <config_path>\n", argv[0]);
    }

    if (file_is_existed(argv[1]) == -1)
    {
        perror("file_is_existed");
        exit(-1);
    }

    if (parse_config(argv[1]) == -1)
    {
        perror("file_is_existed");
        exit(-1);
    }

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(listenfd);
    set_reuse_addr(listenfd);
    pservent = getservbyname("http", "tcp");
    listen_port = pservent->s_port;

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = (listen_port);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    Listen(listenfd, MAX_BACKLOG);

    epollfd = Epoll_create(MAX_EVENTS);
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    Epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    pthread_attr_init(&pthread_attr_detach);
    pthread_attr_setdetachstate(&pthread_attr_detach, PTHREAD_CREATE_DETACHED);

    while (1)
    {
        nfds = Epoll_wait(epollfd, events, MAX_EVENTS, -1);

    }
}

