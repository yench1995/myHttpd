#include "toolkit.h"

map<string, int> config_keyword_map;
/***************************** toolkit function ****************************/

string time_gmt()
{
    time_t now;
    struct tm *time_now;
    string str_time;

    time(&now);
    time_now = localtime(&now);

    switch(time_now->tm_wday)
    {
        case 0:
            str_time += "Sun, ";
            break;
        case 1:
            str_time += "Mon, ";
            break;
        case 2:
            str_time += "Tue, ";
            break;
        case 3:
            str_time += "Wed, ";
            break;
        case 4:
            str_time += "Thu, ";
            break;
        case 5:
            str_time += "Fri, ";
            break;
        case 6:
            str_time += "Sat, ";
            break;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%d ", time_now->tm_mday);
    str_time += string(buf);
    switch(time_now->tm_mon)
    {
        case 0:
            str_time += "Jan ";
            break;
        case 1:
            str_time += "Feb ";
            break;
        case 2:
            str_time += "Mar ";
            break;
        case 3:
            str_time += "Apr ";
            break;
        case 4:
            str_time += "May ";
            break;
        case 5:
            str_time += "Jun ";
            break;
        case 6:
            str_time += "Jul ";
            break;
        case 7:
            str_time += "Aug ";
            break;
        case 8:
            str_time += "Sep ";
            break;
        case 9:
            str_time += "Oct ";
            break;
        case 10:
            str_time += "Nov ";
            break;
        case 11:
            str_time += "Dec ";
            break;
    }
    snprintf(buf, sizeof(buf), "%d", time_now->tm_year + 1900);
    str_time += string(buf);
    snprintf(buf, sizeof(buf), " %d:%d:%d ", time_now->tm_hour, time_now->tm_min, time_now->tm_sec);
    str_time += string(buf);

    str_time += "GMT";
    return str_time;
}

string make_real_url(const string& url)
{
    string real_url, url2;
    int n = 0;

    if((n = url.find()))
}

int Socket(int domain, int type, int protocol)
{
    int listenfd;
    if ((listenfd = socket(domain, type, protocol)) == -1)
    {
        perror("socket");
        exit(-1);
    }
    return listenfd;
}

int Listen(int sockfd, int backlog)
{
    if(listen(sockfd, backlog) == -1)
    {
        perror("listen");
        exit(-1);
    }
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (bind(sockfd, addr, addrlen) == -1)
    {
        perror("bind");
        exit(-1);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int connfd = 0;
    for (;;)
    {
        connfd = accept(sockfd, addr, addrlen);
        if (connfd > 0)
            break;
        else if (connfd == -1)
        {
            if (errno != EAGAIN && errno != ECONNABORTED) //信号问题？
            {
                perror("accept");
                exit(-1);
            }
        }
        else
            continue;
    }
    return connfd;
}

struct servent* Getservbyname(const char *name, const char *proto)
{
    struct servent *pservent;
    if ((pservent = getservbyname(name, proto)) == NULL)
    {
        perror("getservbyname");
        exit(-1);
    }
    return pservent;
}

int Epoll_create(int size)
{
    int epollfd = epoll_create(size);
    if (epollfd == -1)
    {
        perror("epoll_create");
        exit(-1);
    }
    return epollfd;
}

void Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    if (epoll_ctl(epfd, op, fd, event) == -1)
    {
        perror("epoll_ctl");
        exit(-1);
    }
}

int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    while (1)
    {
        int nfds = epoll_wait(epfd, events, maxevents, timeout);
        if (nfds == -1)
        {
            if (errno != EINTR)
            {
                perror("epoll_wait");
                exit(-1);
            }
            else
                continue;
        }
        return nfds;
    }
}

void *Calloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);
    if (ptr == NULL)
    {
        perror("Calloc");
        exit(-1);
    }
    return ptr;
}

void *Malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        perror("Malloc");
        exit(-1);
    }
    return ptr;
}

void Free(void *ptr)
{
    free(ptr);
}

