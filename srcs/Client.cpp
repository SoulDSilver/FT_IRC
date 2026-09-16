#include "Client.hpp"

Client::Client() : fd(-1), nickname(""), username("") {}

Client::Client(int fd, const std::string &password)
    : fd(fd), nickname(""), username(password) {}

Client::Client(const Client &other)
    : fd(other.fd), nickname(other.nickname), username(other.username) {}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        fd = other.fd;
        nickname = other.nickname;
        username = other.username;
    }
    return *this;
}

Client::~Client() {}

int Client::getFd() const
{
    return fd;
}

const std::string &Client::getUsername() const
{
    // TODO: inserir instrução return aqui
    return username;
}

