#include "myHttpd.h"

#define ONEKILO    1024
#define ONEMEGA    1024 * ONEKILO
#define ONEGIGA    1024 * ONEMEGA

#define TIMEOUT    1000*60*4 //millseconds

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

char *get_state_by_codes(int http_codes)
{
    switch (http_codes)
    {
        case OK:
            return ok;
            break;
        case BADREQUEST:
            return badrequest;
            break;
        case FORBIDDEN:
            return forbidden;
            break;
        case NOTFOUND:
            return notfound;
            break;
        case NOIMPLEMENTED:
            return noimplemented;
            break;
    }

    return NULL;
}

int readn(int connfd, char *buff, size_t count)
{
    int nread = 0;
    int n;
    while ((n = read(connfd, buff, ONEMEGA-1)))
        nread += n;
    if (n == -1 && errno != EAGAIN)
        return -1;
    return nread;
}

ssize_t writen(int connfd, const char *buff, size_t count)
{
    int nwrite = count;
    int n;
    while(nwrite > 0)
    {
        n = write(connfd, buff+count-nwrite, nwrite);
        if (n < nwrite)
        {
            if (nwrite == -1 && errno != EAGAIN)
                return -1;
            else
                break;
        }
        nwrite -= n;
    }
    return count-nwrite;
}

int do_http_header(http_header_t *phttphdr, string& out)
{
    char status_line[256] = {0};
    string crlf("\r\n");
    string server("Server: myhttpd\r\n");
    string Public("Public: GET, HEAD\r\n");
    string content_base = "Content-Base: " + domain + crlf;
    string date = "Date:" + time_gmt() + crlf;

    string content_length("Content-Length: ");
    string conteng_location("Content-Location: ");
    string last_modified("Last-Modified: ");

    if (phttphdr == NULL)
    {
        snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n",
                BADREQUEST, get_state_by_codes(BADREQUEST));
        out = status_line + crlf;
        return BADREQUEST;
    }

    string method = phttphdr->method;
    string real_url = make_real_url(phttphdr->url);
    string version = phttphdr->version;
    if (method == "GET" || method == "HEAD")
    {
        if (file_is_existed(real_url.c_str()) == -1)
        {
            snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n",
                    NOTFOUND, get_state_by_codes(NOTFOUND));
            out += (status_line + server + date + crlf);
            return NOTFOUND;
        }
        else
        {
            snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n",
                    OK, get_state_by_codes(OK));
            out += status_line;

            int len = get_file_length(real_url.c_str());
            snprintf(status_line, sizeof(status_line), "%d\r\n", len);
            out += content_length + status_line;
            out += server + content_base + date;
            out += last_modified + get_file_last_modified_time(real_url.c_str()) + crlf + crlf);
        }
    }
    else if (method == "PUT")
    {
        snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n",
                NOIMPLEMENTED, get_state_by_codes(NOIMPLEMENTED));
        out += status_line + server + Public + date + crlf;
        return NOIMPLEMENTED;
    }
    else if (method == "POST")
    {
        snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n",
                NOIMPLEMENTED, get_state_by_codes(NOIMPLEMENTED));
        out += status_line + server + Public + date + crlf;
        return NOIMPLEMENTED;
    }
    else
    {
        snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n",
                BADREQUEST, get_state_by_codes(BADREQUEST));
        out = status_line + crlf;
        return BADREQUEST;
    }

    return OK;
}

void *thread_func(void *param)
{
    thread_num_add1();
    http_header_t *phttphdr = alloc_http_header();

    epollfd_connfd *ptr_epollfd_connfd = (epollfd_connfd*)param;
    int connfd = ptr_epollfd_connfd->connfd;

    //create new epoll for the connection in the thread
    struct epoll_event ev, events[2];
    ev.events = EPOLLIN|EPOLLET;
    ev.data.fd = connfd;
    int epollfd = Epoll_create(2);
    Epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    int nfds = 0;

    pthread_t tid = pthread_self();
    printf("Thread %u is running now !\n", (unsigned int)tid);

    //assign 1m buff for the http request
    char *buff = (char*)Malloc(ONEMEGA);
    bzero(buff, ONEMEGA);

    set_off_tcp_nagle(connfd);
    set_recv_timeo(connfd, 60, 0);

    int n = readn(connfd, buff, ONEMEGA-1);

    if (n != 0)
    {
        string str_http_request(buff, buff+n);
        if (!parse_http_request(str_http_request, phttphdr))
        {
            perror("parse_http_request: parse str_http_request failed)");
            goto exit;
        }

        cout << "The parsed http request" << endl;
        print_http_header(phttphdr);

        string out;
        int http_codes = do_http_header(phttphdr, out);

        cout <<"the back http request:" << endl << out << endl;

        char *out_buf = (char *)Malloc(out.size());
        if (out_buf == NULL)
            goto exit;
        size_t i;
        for (i=0; i != out.size(); ++i)
            out_buf[i] = out[i];
        out_buf[i] = '\0';
        ssize_t nwrite;
        if (http_codes == BADREQUEST||
                http_codes == NOIMPLEMENTED||
                http_codes == NOTFOUND||
                (http_codes == OK && phttphdr->method == "HEAD"))
            nwrite = writen(connfd, out_buf, strlen(out_buf));

        if (http_codes == OK)
        {
            if (phttphdr->method == "GET")
                nwrite = writen(connfd, out_buf, strlen(out_buf));
            string real_url = make_real_url(phttphdr->url);
            int fd = open(real_url.c_str(), O_RDONLY);
            int file_size = get_file_length(real_url.c_str());
            cout << "file size " << file_size << endl;
            nwrite = 0;
            cout <<"sendfile : " << real_url << endl;
        again:
            if ((sendfile(connfd, fd, (off_t*)&nwrite, file_size)) < 0)
                perror("sendfile");
            if (nwrite < file_size)
                goto again;
            cout << "sendfile ok: " << nwrite << endl;
        }
    }
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
        if (nfds == -1 && errno == EINTR)
            continue;

        for (int n = 0; n != nfds; ++n)
        {
            if (events[n].data.fd == listenfd)
            {
                connfd = Accept(listenfd, (struct sockaddr*)&clientaddr, &addrlen);
                set_nonblocking(connfd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                Epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            else
            {
                epollfd_connfd.epollfd = epollfd;
                epollfd_connfd.connfd = events[n].data.fd;
                ev.data.fd = events[n].data.fd;
                Epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &ev);
                pthread_create(&tid, &pthread_attr_detach, &thread_func, (void *)&epollfd_connfd);
            }
        }
    }
    pthread_attr_destroy(&pthread_attr_detach);
    close(listenfd);
    return 0;
}

