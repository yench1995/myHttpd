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

string make_real_url(const string& url) //这个函数不理解
{
    string real_url, url2;

    if( url.find(domain, 0) != string::npos)
        url2 = url.substr(domain.size(), url.size()-domain.size());
    else
        url2 = url;

    if (docroot[docroot.size()-1] == '/')
    {
        if (url2[0] == '/')
            real_url = docroot + url2.erase(0,1);
        else
            real_url = docroot + url2;
    }
    else
    {
        if (url2[0] == '/')
            real_url = docroot + url2;
        else
            real_url = docroot + '/' + url2;
    }

    return real_url;
}

int get_file_length(const char *path)
{
    struct stat buf;
    int ret;
    if ((ret = stat(path, &buf)) == -1)
    {
        perror("get_file_length");
        exit(-1);
    }
    return (int)buf.st_size;
}

string get_file_modified_time(const char *path)
{
    struct stat buf;
    int ret;
    if ((ret = stat(path, &buf)) == -1)
    {
        perror("get_file_modified_time)");
        exit(-1);
    }
    char arr[32] = {0};
    snprintf(arr, sizeof(arr), "%s", ctime(&buf.st_mtime));
    return string(arr, arr+strlen(arr));
}


int parse_config(const char *path)
{
    config_keyword_map.insert(make_pair("docroot", DOCROOT));
    config_keyword_map.insert(make_pair("domain", DOMAIN));

    int ret = 0;
    fstream infile(path, fstream::in);
    string line, word;
    if (!infile)
    {
        printf("%s can't open\n", path);
        infile.close();
        return -1;
    }
    while (getline(infile, line))
    {
        stringstream stream(line);
        stream >> word;
        map<string, int>::const_iterator itr = config_keyword_map.find(word);
        if (itr == config_keyword_map.end())
        {
            printf("can't find keyword\n");
            infile.close();
            return -1;
        }
        switch (itr ->second)
        {
            case DOCROOT:
                stream >> docroot;
                break;
            case DOMAIN:
                stream >> domain;
                break;
            default:
                infile.close();
                return -1;
        }
    }
    infile.close();
    return 0;
}

void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        perror("fcntl: F_GETFL");
        exit(-1);
    }
    flags |= O_NONBLOCK;

    int ret = fcntl(fd, F_SETFL, flags);
    if (ret < 0)
    {
        perror("fcntl");
        exit(-1);
    }
}

void set_reuse_addr(int sockfd)
{
    int on = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1)
    {
        perror("setsockopt: SO_REUSEADDR");
        exit(-1);
    }
}

void set_off_tcp_nagle(int sockfd)
{
    int on = 1;
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    if (ret == -1)
    {
        perror("setsockopt: TCP_NODELAY ON");
        exit(-1);
    }
}

void set_on_tcp_nagle(int sockfd)
{
    int off = 0;
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &off, sizeof(off));
    if (ret == -1)
    {
        perror("setsockopt: TCP_NODELAY OFF");
        exit(-1);
    }
}

void set_on_tcp_cork(int sockfd)
{
    int on = 1;
    int ret = setsockopt(sockfd, SOL_TCP, TCP_CORK, &on, sizeof(on));
    if (ret == -1)
    {
        perror("setsockopt: TCP_CORK ON");
        exit(-1);
    }
}

void set_off_tcp_cork(int sockfd)
{
    int off = 1;
    int ret = setsockopt(sockfd, SOL_TCP, TCP_CORK, &off, sizeof(off));
    if (ret == -1)
    {
        perror("setsockopt: TCP_CORK OFF");
        exit(-1);
    }
}

void set_recv_timeo(int sockfd, int sec, int usec)
{
    struct timeval time = {sec, usec};
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time));
    if (ret == -1)
    {
        perror("setsockopt: SO_RCVTIMEO");
        exit(-1);
    }
}

void set_snd_timeo(int sockfd, int sec, int usec)
{
    struct timeval time = {sec, usec};
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &time, sizeof(time));
    if (ret == -1)
    {
        perror("setsockopt: SO_SNDTIMEO");
        exit(-1);
    }
}
/***************************** toolkit function ****************************/


/***************************** wrapper function ****************************/

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

void Listen(int sockfd, int backlog)
{
    if(listen(sockfd, backlog) == -1)
    {
        perror("listen");
        exit(-1);
    }
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
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

/***************************** wrapper function ****************************/
