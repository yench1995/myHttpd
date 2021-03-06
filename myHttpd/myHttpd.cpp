#include "myHttpd.h"

/*定义HTTP的状态信息*/
const char *ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";

/*网页所在文件夹和域名*/
const char *doc_root = "/home/yench/Github/myHttpd/myHttpd/";
const char *domain = "www.yench.com";

int set_nonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

char *get_time()
{
    time_t now;
    struct tm *time_now;
    std::string str_time;

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
    str_time += std::string(buf);
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
    str_time += std::string(buf);
    snprintf(buf, sizeof(buf), " %d:%d:%d ", time_now->tm_hour, time_now->tm_min, time_now->tm_sec);
    str_time += std::string(buf);

    str_time += "GMT";
    return const_cast<char *>(str_time.c_str());
}


void addfd(int epollfd, int fd, bool one_shot) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP; //EPOLLHUP代表本端挂断
    //EPOLLONESHOT指只能触发某fd上注册的一个可读写事件
    //即只能有一个线程或进程处理同一个描述符
    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    set_nonblocking(fd);
}

void removefd(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void modfd(int epollfd, int fd, int ev) {
    epoll_event event;
    event.data.fd = fd;
    //EPOLLRDHUP指对端关闭连接或shutdown关闭半连接
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void http_conn::close_conn(bool real_close) {
    if (real_close && (m_sockfd != -1)) {
        removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--;
    }
}

void http_conn::init(int sockfd, const sockaddr_in &addr) {
    m_sockfd = sockfd;
    m_address = addr;

    int error = 0;
    //非阻塞connect建立连接时会直接返回EINPROGRESS
    //所以需要检查SO_ERROR值是否返回0来看是否建立成功
    socklen_t len = sizeof(error);
    getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

    addfd(m_epollfd, sockfd, true);
    m_user_count++;

    init();
}

void http_conn::init() {
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;

    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    m_accept_language = 0;
    m_accept = 0;
    m_accept_encoding = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    memset(m_real_file, '\0', 200);
    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}

/*读取一行的内容，返回状态机信息*/
http_conn::LINE_STATUS http_conn::parse_line() {
    char temp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx) {
        temp = m_read_buf[m_checked_idx];
        if (temp == '\r') {
            //如果下一个字符是'\n',则读取一个完整的行
            //否则返回LINE_OPEN，等待下一次读取
            if ((m_checked_idx + 1) == m_read_idx)
                return LINE_OPEN;
            else if (m_read_buf[m_checked_idx+1] == '\n') {
                //把\r\n变为\0\0
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }

            //语法有错
            return LINE_BAD;
        }

        //接着上一次的'\r'继续分析
        else if (temp == '\n') {
            if ((m_checked_idx > 1) && (m_read_buf[m_checked_idx -1] == '\r')) {
                m_read_buf[m_checked_idx-1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }

    return LINE_OPEN;
}

/*循环读取客户数据，直到无数据可读或者对方关闭连接*/
bool http_conn::read() {
    if (m_read_idx >= READ_BUFFER_SIZE)
        return false;

    int nread = 0;
    while (1) {
        nread = recv(m_sockfd, m_read_buf+m_read_idx, READ_BUFFER_SIZE-m_read_idx, 0);
        if (nread == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return false;
        } else if (nread == 0) {
            //返回0表示连接关闭
            return false;
        }

        m_read_idx += nread;
    }
    return true;
}

/*解析HTTP请求行，获得请求方法、URL、以及HTTP版本号*/
http_conn::HTTP_CODE http_conn::parse_request_line(char *text) {
    //strpbrk函数返回两个字符串中首个相同字符的位置
    //检索空白字符和'\t'
    m_url = strpbrk(text, " \t");
    if (!m_url)
        return BAD_REQUEST;
    *m_url++ = '\0';

    char *method = text;

    //当前仅支持GET操作
    if (strcasecmp(method, "GET") == 0)
        m_method = GET;
    else
        return BAD_REQUEST;

    //strspn用来计算前者字符串中有连续几个字符属于后者字符串
    //跳过多于空白字符
    m_url += strspn(m_url, " \t");
    m_version = strpbrk(m_url, " \t");
    if (!m_version)
        return BAD_REQUEST;

    *m_version++ = '\0';
    m_version += strspn(m_version, " \t");
    if (strcasecmp(m_version, "HTTP/1.1") != 0)
        return BAD_REQUEST;

    if (strncasecmp(m_url, "http://", 7) == 0) {
        m_url += 7;
    }

    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

/*解析HTTP请求的头部信息*/
http_conn::HTTP_CODE http_conn::parse_headers(char *text) {
    //遇到空行，表示头部解析完毕
    if (text[0] == '\0') {
        //HEAD方法，仅向服务器请求某个资源的响应头
        if (m_method == HEAD)
            return GET_REQUEST;

        //如果还有body部分，则还有继续读取
        if (m_content_length != 0) {
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }

        return GET_REQUEST;

    //解析Connection
    } else if (strncasecmp(text, "Connection:", 11) == 0) {
        text += 11;
        text += strspn(text, " \t");
        //长连接
        if (strcasecmp (text, "keep-alive") == 0)
            m_linger = true;

    //解析Content-Length
    } else if (strncasecmp(text, "Content-Length:", 15) == 0) {
        text += 15;
        text += strspn(text, " \t");
        m_content_length = atol(text);

    //解析Host
    } else if (strncasecmp(text, "Host:", 5) == 0) {
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    //解析解码方式
    } else if (strncasecmp(text, "Accept-Encoding:", 16)) {
        text += 16;
        text += strspn(text, " \t");
        m_accept_encoding = text;
    } else if (strncasecmp(text, "Accept-Language:", 16)) {
        text += 16;
        text += strspn(text, " \t");
        m_accept_language = text;
    } else if (strncasecmp(text, "Accept:", 7)) {
        text += 16;
        text += strspn(text, " \t");
        m_accept = text;
    } else {
        printf("Unknown header: %s", text);
    }

    printf("parse end\n");
    return NO_REQUEST;
}

/*解析HTTP请求的正文(并没有实现，留坑)*/
http_conn::HTTP_CODE http_conn::parse_content(char *text) {
    if (m_read_idx >= (m_content_length + m_checked_idx)) {
        text[m_content_length] = '\0';
        return GET_REQUEST;
    }

    return NO_REQUEST;
}

/*主解析函数*/
http_conn::HTTP_CODE http_conn::process_read() {
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = 0;

    while (((m_check_state == CHECK_STATE_CONTENT) && (line_status == LINE_OK))
            || ((line_status = parse_line()) == LINE_OK)) {
        text = get_line();
        m_start_line = m_checked_idx;
        printf("get 1 http line: %s\n", text);

        switch (m_check_state) {
            case CHECK_STATE_REQUESTLINE:    //第一个状态，分析请求
            {
                ret = parse_request_line(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            }

            case CHECK_STATE_HEADER:         //第二个状态，分析头部
            {
                ret = parse_headers(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                else if (ret == GET_REQUEST)
                    return do_request();
                break;
            }

            case CHECK_STATE_CONTENT:        //第三个状态，分析正文
            {
                ret = parse_content(text);
                if (ret == GET_REQUEST)
                    return do_request();
                line_status = LINE_OPEN;
                break;
            }

            default:
                return INTERNAL_ERROR;
        }
    }

    return NO_REQUEST;
}

/*
 * 解析url
 * 转化成文件在主机上的地址
 */
void http_conn::make_url(char *url) {
    if (strncasecmp(url, domain, 13) == 0)
        url += 13;
    if (url[0] == 0 || (url[0] == '/' && url[1] == '\0')) {
        strcpy(m_real_file, doc_root);
        strcat(m_real_file, "index.html");
    } else if (url[0] == '/') {
        strcpy(m_real_file, doc_root);
        strcat(m_real_file, url+1);
    } else {
        strcpy(m_real_file, doc_root);
        strcat(m_real_file, url);
    }
}

/*
 * 分析HTTP请求的文件的属性
 * 如果目标正确，则使用mmap将其映射到内存地址m_file_address处
 * 并告诉调用者获取文件成功
 */
http_conn::HTTP_CODE http_conn::do_request() {
    //检测文件的状态是否可访问
    printf("the origin url: %s\n", m_url);
    make_url(m_url);
    printf("the file address: %s\n", m_real_file);
    if (stat(m_real_file, &m_file_stat) < 0)
        return NO_RESOURCE;
    if (!(m_file_stat.st_mode & S_IROTH))
        return FORBIDDEN_REQUEST;
    if (S_ISDIR(m_file_stat.st_mode))
        return BAD_REQUEST;

    //建立内存映射
    int fd = open(m_real_file, O_RDONLY);
    m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

/*对内存映射去执行munmap操作*/
void http_conn::unmap() {
    if (m_file_address) {
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = 0;
    }
}

/*写HTTP响应*/
bool http_conn::write() {
    int temp = 0;
    int bytes_have_send = 0;
    int bytes_to_send = m_write_idx;
    if (bytes_to_send == 0) {
        modfd(m_epollfd, m_sockfd, EPOLLIN);
        init();
        return true;
    }

    while (1) {
        //聚集写，由多个缓冲区向sockfd写入
        temp = writev(m_sockfd, m_iv, m_iv_count);
        if (temp <= -1) {
            if (errno == EAGAIN){
            //缓冲区满，则下次再写
                modfd(m_epollfd, m_sockfd, EPOLLOUT);
                return true;
            }

            unmap();
            return false;
        }

        bytes_have_send += temp;
        if (bytes_to_send <= bytes_have_send) {
            unmap();
            //长连接则重置http_conn结构，否则返回false使其关闭
            if (m_linger) {
                init();
                modfd(m_epollfd, m_sockfd, EPOLLIN);
                return true;
            } else {
                return false;
            }
        }
    }
}

/*往写缓冲区写入待发送的头部信息*/
bool http_conn::add_response(const char *format, ...) {
    if (m_write_idx >= WRITE_BUFFER_SIZE)
        return false;
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE-1-m_write_idx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE-1-m_write_idx))
        return false;

    m_write_idx += len;
    va_end(arg_list);
    return true;
}

/*添加状态行*/
bool http_conn::add_status_line(int status, const char *title) {
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

/*添加头部*/
bool http_conn::add_headers(int content_len) {
    add_content_type();
    add_content_length(content_len);
    add_linger();
    add_blank_line();
}

/*添加长度信息*/
bool http_conn::add_content_length(int content_len) {
    return add_response("Content-Length: %d\r\n", content_len);
}

/*添加连接信息*/
bool http_conn::add_linger() {
    return add_response("Connection: %s\r\n", (m_linger == true) ? "keep-alive" : "close");
}


/*添加时间*/
bool http_conn::add_date(const char* time) {
    return add_response("Date: %s\r\n", time);
}

/*添加类型格式*/
bool http_conn::add_content_type() {
    return add_response("Content-Type: text/html;charset=ISO-8859-1\r\n");
}

/*添加最后修改时间*/
bool http_conn::add_lastmodified(const char *last_mod_time) {
    return add_response("Last-Modified: %s\r\n", last_mod_time);
}

/*添加空白行*/
bool http_conn::add_blank_line() {
    return add_response("%s", "\r\n");
}

/*添加content*/
bool http_conn::add_content(const char *content) {
    return add_response("%s", content);
}

/*根据服务器处理HTTP请求的结果，决定返回给客户端的内容*/
bool http_conn::process_write(HTTP_CODE ret) {
    switch (ret) {
        case INTERNAL_ERROR:
        {
            add_status_line(500, error_500_title);
            add_headers(strlen(error_500_form));
            if (!add_content(error_500_form))
                return false;
            break;
        }

        case BAD_REQUEST:
        {
            add_status_line(400, error_400_title);
            add_headers(strlen(error_400_form));
            if (!add_content(error_400_form))
                return false;
            break;
        }

        case NO_RESOURCE:
        {
            add_status_line(404, error_404_title);
            add_headers(strlen(error_404_form));
            if (!add_content(error_404_form))
                return false;
            break;
        }

        case FORBIDDEN_REQUEST:
        {
            add_status_line(403, error_403_title);
            add_headers(strlen(error_403_form));
            if (!add_content(error_403_form))
                return false;
            break;
        }

        case FILE_REQUEST:
        {
            add_status_line(200, ok_200_title);
            if (m_file_stat.st_size != 0) {
                add_headers(m_file_stat.st_size);
                m_iv[0].iov_base = m_write_buf;
                m_iv[0].iov_len = m_write_idx;
                m_iv[1].iov_base = m_file_address;
                m_iv[1].iov_len = m_file_stat.st_size;
                m_iv_count = 2;
                return true;
            } else {
                const char *ok_string = "<html><body></body></html>";
                add_headers(strlen(ok_string));
                if (!add_content(ok_string))
                    return false;
            }
        }

        default:
        {
            return false;
        }
    }
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_idx;
    m_iv_count = 1;
    return true;
}

/*线程函数的主调用入口，由它来处理HTTP请求的整个流程*/
void http_conn::process() {
    printf("starting process\n");
    HTTP_CODE read_ret = process_read();
    printf("read end\n");
    if (read_ret == NO_REQUEST) {
        modfd(m_epollfd, m_sockfd, EPOLLIN);
        return;
    }

    bool write_ret = process_write(read_ret);

    //非长连接
    if (!write_ret)
        close_conn();

    modfd(m_epollfd, m_sockfd, EPOLLOUT);

}


