#include "Channel.hpp"
#include "Server.hpp"

Channel::Channel(const string &name, Server &server,
				 const Client &client) : server(server)
{
	this->name = name;
	this->clients.insert(make_pair(client.getFd(), client));
	this->operators.insert(make_pair(client.getFd(), client));
}

Channel::Channel(const Channel &other)
	: name(other.name), server(other.server),
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

bool Channel::have_any_client() const
{
    return clients.size() == 0;
}

void Channel::addClient(const Client &client)
{
	clients.insert(make_pair(client.getFd(), client));
}

void Channel::removeClient(const Client &client)
{
	clients.erase(client.getFd());
	if (isOperator(client.getFd()))
		removeOperator(client);
}

bool Channel::isOperator(int fd) const
{
	return operators.count(fd) == 1;
}

void Channel::listclients() const
{
	for (map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
		cout << "Client FD: " << it->first << ", Nick: " << it->second.getNick() << ", Username: " << it->second.getUsername() << endl;
	if (clients.empty())
		cout << "No clients in the channel." << endl;
}

void Channel::addOperator(const Client &client)
{
	operators.insert(make_pair(client.getFd(), client));
}

void Channel::removeOperator(const Client &client)
{
	operators.erase(client.getFd());
}

void Channel::broadcastMessage(const string &message, int senderFd) const
{
	string sms;
	map<int, Client>::const_iterator sender = clients.find(senderFd);
	if (sender == clients.end())
		return;
	for (map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->first != senderFd)
		{
			sms = ":" + sender->second.getNick() + "!" + sender->second.getUsername() + "@localhost PRIVMSG " + this->name + " :" + message + "\r\n";
			send(it->first, sms.c_str(), sms.length(), 0);
		}
	}
}

//  /connect localhost 1024 44