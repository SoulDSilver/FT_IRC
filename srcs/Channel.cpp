#include "Channel.hpp"

Channel::Channel() : name("") {}

Channel::Channel(const std::string &name) : name(name) {}

Channel::Channel(const Channel &other) : name(other.name), clients(other.clients) {}

Channel &Channel::operator=(const Channel &other)
{
    if (this != &other)
    {
        name = other.name;
        clients = other.clients;
    }
    return *this;
}

Channel::~Channel() {}

const std::map<int, Client> &Channel::getClients() const
{
    // TODO: inserir instrução return aqui
}

void Channel::addClient(const Client &client)
{
    (void)client;
}

void Channel::removeClient(const Client &client)
{
    (void)client;
}
