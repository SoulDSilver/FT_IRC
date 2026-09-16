#pragma once

#include "irc.hpp"

class Server
{
private:
    int listen_port;
    const std::string password;
    int listen_fd;
    Server();
    int listen_socket(int listen_port);

public:
    Server(int port);
    Server(const Server &other);
    Server &operator=(const Server &other);
    ~Server();
    void run();
    int getListenPort() const;
    int getListenFd() const;
};
