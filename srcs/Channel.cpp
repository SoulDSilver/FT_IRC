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

const string &Channel::getPassword() const
{
	return (password);
}

const map<int, Client> &Channel::getClients() const
{
	return (clients);
}

const map<int, Client> &Channel::getOperators() const
{
	return (operators);
}

const vector<string> &Channel::getSettings() const
{
	return (settings);
}

const vector<int> &Channel::getInvitedUsers() const
{
	return (invitedUsers);
}

void Channel::setPassword(const string &password)
{
	this->password = password;
}

bool Channel::have_any_client() const
{
	return clients.empty();
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

void Channel::addSetting(const string &setting)
{
	if (find(settings.begin(), settings.end(), setting) == settings.end())
		settings.push_back(setting);
}

void Channel::removeSetting(const string &setting)
{
	vector<string>::iterator it = find(settings.begin(), settings.end(), setting);
	if (it != settings.end())
		settings.erase(it);
}

void Channel::addInvitedUser(int a)
{
	if (find(invitedUsers.begin(), invitedUsers.end(), a) == invitedUsers.end())
		invitedUsers.push_back(a);
}

void Channel::removeInvitedUser(const int &a)
{
	vector<int>::iterator it =  find(invitedUsers.begin(), invitedUsers.end(), a);
	if (it != invitedUsers.end())
		invitedUsers.erase(it);
}

void Channel::sendJoinMessages(const Client &client) const
{
	string host = client.getIpAddr();
	if (host.empty())
		host = "host";
	string prefix = ":" + client.getNick() + "!" + client.getUsername() + "@" + host;
	string names;

	for (map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (!names.empty())
			names += " ";
		if (isOperator(it->first))
			names += "@";
		names += it->second.getNick();
	}

	string messages = prefix + " JOIN :" + name + "\r\n";
	messages += ":" + server.getName() + " 331 " + client.getNick() + " " + name + " :No topic is set\r\n";
	messages += ":" + server.getName() + " 353 " + client.getNick() + " = " + name + " :" + names + "\r\n";
	messages += ":" + server.getName() + " 366 " + client.getNick() + " " + name + " :End of /NAMES list\r\n";
	send(client.getFd(), messages.c_str(), messages.length(), 0);
}

void Channel::sendPartMessage(const Client &client, const string &reason) const
{
	string host = client.getIpAddr();
	if (host.empty())
		host = "host";
	string message = ":" + client.getNick() + "!" + client.getUsername() + "@" + host;
	message += " PART " + name;
	if (!reason.empty())
		message += " :" + reason;
	message += "\r\n";

	for (map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
		send(it->first, message.c_str(), message.length(), 0);
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

bool Channel::isClientPresent(int fd) const
{
	return (this->clients.find(fd) != this->clients.end());
}

//  /connect localhost 1024 44