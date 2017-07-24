#include "parse.h"

void print_http_header_header(const m_header& head)
{
    if (!head.empty())
    {
        m_header::const_iterator itr = head.begin();
        while (itr != head.end())
        {
            cout << itr->first << ": " << itr->second << endl;
            ++itr;
        }
    }
}

void print_http_header(http_header_t *phttphdr)
{
    if (phttphdr == NULL)
    {
        perror("phttphdr == NULL");
        return;
    }

    cout << phttphdr->method << " " << phttphdr->url << " " << phttphdr->version << endl;
    print_http_header_header(phttphdr -> header);
    cout << endl << phttphdr->body << endl;
}

http_header_t *alloc_http_header()
{
    http_header_t *phttphdr = new http_header_t;
    if (phttphdr == NULL)
    {
        perror("alloc_http_header");
        exit(-1);
    }
    return phttphdr;
}

void free_http_header(http_header_t *phttphdr)
{
    if (phttphdr == NULL)
        return;
    delete phttphdr;
}

bool parse_http_request(const string& http_request, http_header_t *phttphdr)
{
    if (http_request.empty())
    {
        perror("parse_http_request: http_request is empty");
        return false;
    }
    if (phttphdr == NULL)
    {
        perror("parse_http_request: phttphdr is NULL");
        return false;
    }

    string crlf("\r\n"), crlfcrlf("\r\n\r\n");
    size_t prev = 0, next = 0;

    if ((next = http_request.find(crlf, prev)) != string::npos)
    {
        string first_line = http_request.substr(prev, next-prev);
        prev = next;
        stringstream sstream(first_line);
        sstream >> (phttphdr->method);
        sstream >> (phttphdr->url);
        sstream >> (phttphdr->version);
    }
    else
    {
        perror("parse_http_request: http_request has not a \\r\\n");
        return false;
    }

    size_t pos_crlfcrlf = http_request.find(crlfcrlf, prev);
    if (pos_crlfcrlf == string::npos)
    {
        perror("parse_http_request: http_request has no a \\r\\n\\r\\n");
        return false;
    }

    string buff_line, key, value;
    while (1)
    {
        next = http_request.find(crlf, prev+2);

        if (next <= pos_crlfcrlf)
        {
            buff_line = http_request.substr(prev+2, next-prev-2);
            size_t end = 0;
            for (; isblank(buff_line[end]); ++end)
                ;
            int beg = end;
            for (; buff_line[end] != ':' && !isblank(buff_line[end]); ++end)
                ;
            key = buff_line.substr(beg, end-beg);
            for (; !isalpha(buff_line[end]) && !isdigit(buff_line[end]); ++end)
                ;
            beg = end;
            for (; end != next; ++end)
                ;
            value = buff_line.substr(beg, end-beg);
            phttphdr->header.insert(make_pair(key, value));

            prev = next;
        }
        else
            break;
    }

    phttphdr->body = http_request.substr(pos_crlfcrlf+4, http_request.size()-pos_crlfcrlf-4);

    return true;
}

string get_value_from_http_header(const string& key, const m_header& header)
{
    if (header.empty())
        return "";
    m_header::const_iterator itr = header.find(key);
    if (itr == header.end())
        return "";
    return itr->second;
}

/*
int main(int argc, char **argv)
{
    http_header_t *phttphdr = alloc_http_header();
    string http_request = "GET /home/yench HTTP1.1\r\n\
Length: 8080\r\n\
Date: July Sat 2017\r\n\
\r\n\
<html>\n\
hello, yench\n\
</html>";

    cout << "http_request size:" << http_request.size() << endl;
    parse_http_request(http_request, phttphdr);

    cout << phttphdr->method << " " << phttphdr->url <<" " << phttphdr->version << endl;
    string str;
    str = get_value_from_http_header("Lenght", phttphdr->header);
    cout <<"Length: " << str << endl;
    str = get_value_from_http_header("Date", phttphdr->header);
    cout <<"Date: " << str << endl;
    cout << phttphdr->body << endl;

    free_http_header(phttphdr);
    return 0;
}
*/

