#ifndef _MYHTTPD_H_
#define _MYHTTPD_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <strings.h>
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
#include <sys/sendfile.h>

#include "toolkit.h"
#include "parse.h"

using namespace std;

typedef struct _epollfd_connfd
{
    int epollfd;
    int connfd;
}_epollfd_connfd;

/******************************** constant define ******************************/

#define MAX_EVENTS 1024 //the most epoll connected events
#define MAX_BACKLOG 100 //the most listening queue

/******************************** constant define ******************************/

/******************************** configuration file******************************/

string domain("");
string docroot("");

/******************************** configuration file******************************/

/******************************** defination of MIME******************************/

typedef struct mime_node
{
    const char *type;
    const char *value;
} mime_node;

mime_node mime[] = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {NULL ,NULL}
};

/******************************** defination of MIME******************************/
