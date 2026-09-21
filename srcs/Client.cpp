#include "Client.hpp"

Client::Client() : fd(-1), ipAddr(""), username("") {}



Client::Client(const Client &other)
    : fd(other.fd), ipAddr(other.ipAddr), username(other.username) {}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        fd = other.fd;
        ipAddr = other.ipAddr;
        username = other.username;
    }
    return *this;
}

Client::~Client() {}

int Client::getFd() const
{
    return fd;
}
void Client::setFd(int fd)
{
    this->fd = fd;
}

const std::string &Client::getIpAddr() const
{
    return ipAddr;
}
void Client::setIpAddr(const std::string &ipAddr)
{
    this->ipAddr = ipAddr;
}

const std::string &Client::getUsername() const
{
    return username;
}
void Client::setUsername(const std::string &username)
{
    this->username = username;
}

