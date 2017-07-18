#include "wrap_func.h"

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
