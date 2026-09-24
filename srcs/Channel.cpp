#include "Channel.hpp"
#include "Server.hpp"

Channel::Channel(const string &name, Server &server,
				 const Client &client) : server(server)
{
	this->name = name;
	this->clients.insert(make_pair(client.getFd(), client));
	operators.push_back(client.getFd());
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
	cout << client;
	// clients.at(client.getFd()) = client;
	clients.insert(make_pair(client.getFd(), client));
	cout << client;
}

void Channel::removeClient(const Client &client)
{
	clients.erase(client.getFd());
}

void Channel::listclients() const
{
	for (map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
		cout << "Client FD: " << it->first << ", Nick: " << it->second.getNick() << ", Username: " << it->second.getUsername() << endl;
	if (clients.empty())
		cout << "No clients in the channel." << endl;
}

void Channel::broadcastMessage(const string &message, int senderFd) const
{
	string sms;
	map<int, Client>::const_iterator sender = clients.find(senderFd);
	if (sender == clients.end())
		return;
	for (map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		cout << it->first << " " << senderFd << " testes" << endl;
		if (it->first != senderFd)
		{
			sms = ":" + sender->second.getNick() + "!" + sender->second.getUsername() + "@localhost PRIVMSG " + this->name + " :" + message + "\r\n";
			send(it->first, sms.c_str(), sms.length(), 0);
		}
	}
}

//  /connect localhost 1024 44