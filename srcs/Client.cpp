#include "Client.hpp"

Client::Client() : fd(-1), ipAddr(""), username("") {}

Client::Client(const Client &other)
{
    *this = other; 
}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        fd = other.fd;
        inbuff = other.inbuff;   
        outbuff = other.outbuff; 
        ipAddr = other.ipAddr;
        nick = other.nick;
        username = other.username;
    }
    return *this;
}

bool Client::operator==(const Client &other) const
{
    return (this->fd == other.fd);
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

ostream &operator<<(ostream &stream, const Client &cl)
{
    stream << "Cliente [FD: " << cl.getFd()
           << " | Nick: " << (cl.getNick().empty() ? "<sem_nick>" : cl.getNick())
           << " | User: " << (cl.getUsername().empty() ? "<sem_user>" : cl.getUsername())
           << " | IP: " << cl.getIpAddr()
           << "]";

    return stream;
}
