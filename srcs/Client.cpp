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

const string &Client::getIpAddr() const
{
    return ipAddr;
}
void Client::setIpAddr(const string &ipAddr)
{
    this->ipAddr = ipAddr;
}

const string &Client::getUsername() const
{
    return username;
}
string Client::getNick() const
{
    return nick;
}
void Client::setNick(const string &nick)
{
    this->nick = nick;
}
void Client::setUsername(const string &username)
{
    this->username = username;
}

