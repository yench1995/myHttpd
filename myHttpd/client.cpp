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
#include <arpa/inet.h>
#include <iostream>

#include "toolkit.h"
using namespace std;

int main(int argc, char **argv)
{
    int sockfd, n;
    struct sockaddr_in servaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: client <IP adddress>\n");
        return 1;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "sock error\n");
        return 1;
    }
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        fprintf(stderr, "inet_pton error for %s\n", argv[1]);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "connect error\n");
        return 1;
    }

    printf("Starting send:\n");
    char buff[] = "GET / HTTP/1.1\r\nAccept-Language: zh-cn\r\n\r\n";
    write(sockfd, buff, sizeof(buff));
    printf("Finishing send:\n");
    printf("Starting read:\n");
    char arr[102400];
    int in = 0, nread = 0;
    while ((in = read(sockfd, arr+nread, sizeof(arr))) > 0)
    {
        cout << nread << endl;
        nread += in;
        if (nread == 11510)
            break;
    }
    arr[nread] = '\0';
    printf("myHttpd web sending data: \n%s\n%d\n", arr, nread);
    printf("Finishing reading!\n");
    return 0;
}
