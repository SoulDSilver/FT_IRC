#include "Channel.hpp"


Channel::Channel(const std::string &name, Server &server, Client &client):server(server)
{
    this->name = name;
    this->clients[client.getFd()] = client;
	operators.push_back(client.getUsername());
}

Channel::Channel(const Channel &other) : name(other.name), server(other.server),
	clients(other.clients), operators(other.operators)
{
}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		name = other.name;
		clients = other.clients;
		operators = other.operators;
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
	clients.insert(std::make_pair(client.getFd(), client));
}

void Channel::removeClient(const Client &client)
{
	clients.erase(client.getFd());
}

const std::string &Channel::getName() const
{
	return (this->name);
}
