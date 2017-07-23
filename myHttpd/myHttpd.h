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
#include <unordered_map>

#include "toolkit.h"
#include "parse.h"

using namespace std;

typedef struct epollfd_connfd
{
    int epollfd;
    int connfd;
} epollfd_connfd;

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

inline const char* mime_type2value(const char *type)
{
    for (int i = 0; mime[i].type != NULL; ++i)
    {
        if (strcmp(type, mime[i].type) == 0)
            return mime[i].value;
    }
    return NULL;
}

/******************************** defination of MIME ******************************/


/******************************** HTTP status code ******************************/
#define CONTINUE          100       //收到了请求的起始部分，客户端应继续请求

#define OK                200       //服务器已经成功处理请求
#define ACCEPTED          202       //请求已接受，服务器尚未处理

#define MOVED             301       //请求的URL已移走，响应应该包含Location URL
#define FOUND             302       //请求的URL临时移走，响应应该包含Location URL
#define SEEOTHER          303       //告诉客户端应该用另一个URL获取资源，响应应该包含Location
#define NOTMODIFIED       304       //资源未发生变化

#define BADREQUEST        400       //客户端发送了一条异常请求
#define FORBIDDEN         403       //服务器拒绝请求
#define NOTFOUND          404       //URL未找到

#define ERROR             500       //服务器出错
#define NOIMPLEMENTED     501       //服务器不支持当前请求所需要的某个功能
#define BADGATEWAY        502       //作为代理或网关使用的服务器遇到了来自响应链中上游的无效响应
#define SRVUNAVILABLE     503       //服务器目前无法提供请求服务，过一段时间后可以恢复



char ok[] = "OK";
char badrequest[]  = "Bad Request";
char forbidden[] = "Forbidden";
char notfound[] = "Not Found";
char noimplemented[] = "No implemented";

inline char *get_state_by_codes(int http_codes);



/******************************** HTTP status code ******************************/

/******************************** HTTP request header *****************************/
#define ACCEPTRANGE_HEAD      "Accept-Range"
#define AGC_HEAD              "Age"
#define ALLOW_HEAD            "Allow"
#define CONTENTBASE_HEAD      "Content-Base"
#define CONTENTLENGTH_HEAD    "Content-Length"
#define CONTENTLOCATION_HEAD  "Content-Location"
#define CONTENTRANGE_HEAD     "Content-Range"
#define CONTENTTYPE_HEAD      "Content-Type"
#define DATE_HEAD             "Date"
#define EXPIRES_HEAD          "Expires"
#define LAST_MODIFIED_HEAD    "last-Modified"
#define LOCATION_HEAD         "Location"
#define PUBLIC_HEAD           "Public"
#define RANGE_HEAD            "Range"
#define SERVER_HEAD           "Server"

/******************************** HTTP request header *****************************/

#endif
