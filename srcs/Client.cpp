#include "Client.hpp"

Client::Client() : Fd(-1), IpAddr(""), Username("") {}

Client::Client(const Client &other)
    : Fd(other.Fd), IpAddr(other.IpAddr), Username(other.Username) {}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        Fd = other.Fd;
        IpAddr = other.IpAddr;
        Username = other.Username;
    }
    return *this;
}

Client::~Client() {}

int Client::getFd() const
{
    return Fd;
}
void Client::setFd(int fd)
{
    this->Fd = fd;
}

const string &Client::getIpAddr() const
{
    return IpAddr;
}
void Client::setIpAddr(const string &ipAddr)
{
    this->IpAddr = ipAddr;
}

const string &Client::getUsername() const
{
    return Username;
}
string Client::getNick() const
{
    return Nick;
}
void Client::setNick(const string &nick)
{
    this->Nick = nick;
}
void Client::setUsername(const string &username)
{
    this->Username = username;
}

