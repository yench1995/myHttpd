#ifndef _TOOLKIT_H_
#define _TOOLKIT_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/stat.h>

using namespace std;
extern string docroot;
#define DOCROOT 1
extern string domain;
#define DOMAIN 2

/******************** Tool function *************************/

string time_gmt();
/*
 * get the GMT system time
 */

string make_real_url(const string& url);
/*
 * construct the real url according to url and docroot
 */

int parse_config(const char *path);
/*
 * return -1 means parse false, return 0 means success
 */

int get_file_length(const char *path);

string get_file_last_modified_time(const char *path);

void init_init_config_keyword_map();

void set_nonblocking(int fd);

void set_reuse_addr(int sockfd);

void set_off_tcp_nagle(int sockfd);

void set_on_tcp_nagle(int sockfd);

void set_off_tcp_cork(int sockfd);

void set_on_tcp_cork(int sockfd);

void set_recv_timeo(int sockfd, int sec, int usec);

void set_set_snd_timeo(int sockfd, int sec, int usec);

inline int file_is_existed(const char *path)
{
    int ret = open(path, O_RDONLY | O_EXCL);
    close(ret);
    return ret;
}
/******************** Tool function *************************/



/******************** wraper function of syscall *************************/

int Socket(int domain, int type, int protocol);
void Listen(int sockfd, int backlog);
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
struct servent * Getservbyname(const char *name, const char *proto);
int Epoll_create(int size);
void Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
void *Calloc(size_t nmemb, size_t size);
void *Malloc(size_t size);
void Free(void *ptr);
/******************** wraper function of syscall *************************/

#endif
