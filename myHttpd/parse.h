#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include <utility>
#include <sstream>
#include <ctype.h>
#include <iostream>

using namespace std;

typedef map<string, string> m_header;

typedef struct http_header_t
{
    string method;
    string url;
    string version;

    m_header header;
    string body;
} http_header_t;

void print_http_header_header(const m_header& head);

void print_http_header(http_header_t *phttphdr);

http_header_t *alloc_http_header();

void free_http_header(http_header_t *phttphdr);

bool parse_http_request(const string& http_request, http_header_t *phttphdr);

string get_value_from_http_header(const string& key, const m_header& header);
