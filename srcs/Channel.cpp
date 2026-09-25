#include "Channel.hpp"

Channel::Channel(const string &name, Server &server,
	Client &client) : ServerRef(server)
{
	this->Name = name;
	this->Clients[client.getFd()] = client;
	Operators.push_back(client.getFd());
}

Channel::Channel(const Channel &other) : Name(other.Name),
	ServerRef(other.ServerRef), Clients(other.Clients), Operators(other.Operators)
{
}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		this->Name = other.Name;
		this->Clients = other.Clients;
		this->Operators = other.Operators;
	}
	return (*this);
}

bool Channel::operator==(const Channel &other)
{
	return (this->Name == other.Name);
}

Channel::~Channel()
{
}

const map<int, Client> &Channel::getClients() const
{
	return (Clients);
}

void Channel::addClient(const Client &client)
{
	Clients.insert(std::make_pair(client.getFd(), client));
}

void Channel::removeClient(const Client &client)
{
	Clients.erase(client.getFd());
}

const std::string &Channel::getName() const
{
	return (this->Name);
}
