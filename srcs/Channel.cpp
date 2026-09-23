#include "Channel.hpp"
#include "Server.hpp"

Channel::Channel(const string &name, Server &server,
	Client &client) : server(server)
{
	this->name = name;
	this->clients[client.getFd()] = client;
	operators.push_back(client.getFd());
}

Channel::Channel(const Channel &other) : name(other.name),
	server(other.server), clients(other.clients), operators(other.operators)
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

bool Channel::operator==(const Channel &other)
{
	return (this->name == other.name);
}

Channel::~Channel()
{
}

const string &Channel::getName() const
{
	return (name);
}

const map<int, Client> &Channel::getClients() const
{
	return (clients);
}

void Channel::addClient(const Client &client)
{
	clients.at(client.getFd()) = client;
}

void Channel::removeClient(const Client &client)
{
	clients.erase(client.getFd());
}

void Channel::broadcastMessage(const string &message, int senderFd) const
{
	string sms;
	for (map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->first != senderFd)
		{
			sms = ":" + server.getName()+ " :" + clients.at(senderFd).getNick() + "!" + clients.at(senderFd).getUsername() + "@localhost " + message;
			send(it->first, sms.c_str(), sms.length(), 0);
		}
	}
}
