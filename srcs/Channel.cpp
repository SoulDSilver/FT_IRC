#include "Channel.hpp"

Channel::Channel() : name("")
{
}


Channel::Channel(const std::string &name, Server *server, Client &client)
{
    this->name = name;
    this->server = server;
    this->clients[client.getFd()] = client;
	operators.push_back(client.getUsername());
}

Channel::Channel(const Channel &other) : name(other.name),
	clients(other.clients), operators(other.operators)
{
}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		name = other.name;
		clients = other.clients;
	}
	return (*this);
}

Channel::~Channel()
{
}

const std::map<int, Client> &Channel::getClients() const
{
	return (clients);
}

void Channel::addClient(const Client &client)
{
	clients[client.getFd()] = client;
}

void Channel::removeClient(const Client &client)
{
	clients.erase(client.getFd());
}
