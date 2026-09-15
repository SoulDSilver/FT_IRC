#include "Server.hpp"

static int forward_port;

template <typename T>
static T max(T a, T b)
{
    return (a > b) ? a : b;
}

static int listen_socket(int listen_port)
{
    struct sockaddr_in addr;
    int lfd;
    int yes;

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket");
        return -1;
    }

    yes = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
                   &yes, sizeof(yes)) == -1)
    {
        perror("setsockopt");
        close(lfd);
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(listen_port);
    addr.sin_family = AF_INET;
    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        close(lfd);
        return -1;
    }

    printf("accepting connections on port %d\n", listen_port);
    listen(lfd, 10);
    return lfd;
}
