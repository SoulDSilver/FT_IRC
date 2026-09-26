#include "Client.hpp"

Client::Client() : Fd(-1), IpAddr(""), Username(""),
    PasswordAccepted(false) {}

Client::Client(const Client &other)
    : Fd(other.Fd), IpAddr(other.IpAddr), Username(other.Username),
    PasswordAccepted(other.PasswordAccepted) {}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        Fd = other.Fd;
        IpAddr = other.IpAddr;
        Username = other.Username;
        PasswordAccepted = other.PasswordAccepted;
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

void Client::setPasswordAccepted()
{
    this->PasswordAccepted = true;
}

bool Client::isPasswordAccepted() const
{
    return (this->PasswordAccepted);
}

void Client::appendInput(const string &data)
{
    this->Inbuff += data;
}

const string &Client::getInbuff() const
{
    return (this->Inbuff);
}

bool Client::extractLine(string &line)
{
    size_t end = this->Inbuff.find('\n');
    if (end == string::npos)
        return (false);

    line = this->Inbuff.substr(0, end);
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
    this->Inbuff.erase(0, end + 1);
    return (true);
}

