#include "Channel.hpp"
#include "Server.hpp"

Channel::Channel(const string &name, Server &server,
				 const Client &client) : server(server)
{
	settings.insert(make_pair("Name", name));
	settings.insert(make_pair("Password", ""));
	settings.insert(make_pair("Topic", ""));

	settings.insert(make_pair("Permission", "Public"));

	Limit = 0;
	InviteOnly = false;
	OnlyOperators = false;
	TopicRestricted = false;
	this->clients.insert(make_pair(client.getFd(), client));
	this->clientsByNick.insert(make_pair(client.getNick(), client.getFd()));
	this->operators.insert(make_pair(client.getFd(), client));
}

Channel::Channel(const Channel &other)
	: settings(other.settings), server(other.server),
	  clients(other.clients), operators(other.operators)
{
}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		settings = other.settings;
		clients = other.clients;
	}
	return (*this);
}

bool Channel::operator==(const Channel &other)
{
	return (this->settings.find("Name") != this->settings.end() &&
			this->settings.at("Name") == other.settings.at("Name"));
}

Channel::~Channel()
{
}

const string &Channel::getName() const
{
	return (settings.at("Name"));
}

const string &Channel::getPassword() const
{
	return (settings.at("Password"));
}

const map<int, Client> &Channel::getClients() const
{
	return (clients);
}

const map<int, Client> &Channel::getOperators() const
{
	return (operators);
}

const map<string, string> &Channel::getSettings() const
{
	return (settings);
}

const vector<int> &Channel::getInvitedUsers() const
{
	return (invitedUsers);
}

void Channel::setPassword(const string &password)
{
	settings.at("Password") = password;
}

bool Channel::have_any_client() const
{
	return clients.empty();
}

void Channel::addClient(const Client &client)
{
	clients.insert(make_pair(client.getFd(), client));
	clientsByNick.insert(make_pair(client.getNick(), client.getFd()));
}

void Channel::removeClient(const Client &client)
{
	clients.erase(client.getFd());
	clientsByNick.erase(client.getNick());

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

void Channel::addSetting(const string &setting, const string &value)
{
	settings.insert(make_pair(setting, value));
}

void Channel::removeSetting(const string &setting)
{
	if (settings.count(setting))
		settings.erase(setting);
}

void Channel::addInvitedUser(int a)
{
	if (find(invitedUsers.begin(), invitedUsers.end(), a) == invitedUsers.end())
		invitedUsers.push_back(a);
}

void Channel::removeInvitedUser(const int &a)
{
	vector<int>::iterator it = find(invitedUsers.begin(), invitedUsers.end(), a);
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

	string messages = prefix + " JOIN :" + settings.at("Name") + "\r\n";
	messages += ":" + server.getName() + " 331 " + client.getNick() + " " + settings.at("Name") + " :No topic is set\r\n";
	messages += ":" + server.getName() + " 353 " + client.getNick() + " = " + settings.at("Name") + " :" + names + "\r\n";
	messages += ":" + server.getName() + " 366 " + client.getNick() + " " + settings.at("Name") + " :End of /NAMES list\r\n";
	send(client.getFd(), messages.c_str(), messages.length(), 0);
}

void Channel::sendPartMessage(const Client &client, const string &reason) const
{
	string host = client.getIpAddr();
	if (host.empty())
		host = "host";
	string message = ":" + client.getNick() + "!" + client.getUsername() + "@" + host;
	message += " PART " + settings.at("Name");
	if (!reason.empty())
		message += " :" + reason;
	message += "\r\n";

	for (map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
		send(it->first, message.c_str(), message.length(), 0);
}

void Channel::broadcastMessage(const string &message, const Client &sender) const
{
	string sms;
	int senderFd = sender.getFd();
	if (clients.find(sender.getFd()) == clients.end())
		return;
	sms = ":" + sender.getNick() + "!" + sender.getUsername() + "@" + sender.getIpAddr() + " PRIVMSG " + settings.at("Name") + " :" + message + "\r\n";
	for (map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
		if (it->first != senderFd)
			send(it->first, sms.c_str(), sms.length(), 0);
}

bool Channel::isClientPresent(int fd) const
{
	return (this->clients.find(fd) != this->clients.end());
}

void Channel::sendMessage(const string &message, int senderFd) const
{
	string sms;
	cout << "MSG" << endl;
	map<int, Client>::const_iterator sender = clients.find(senderFd);
	if (sender == clients.end())
		return;
	sms = ":" + sender->second.getNick() + " :" + message + "\r\n";
	send(senderFd, sms.c_str(), sms.length(), 0);
}



const Client *Channel::findClientByNick(const string &nick) const
{
	if(nick.empty())
		return NULL;
   map<std::string, int>::const_iterator nickIt = clientsByNick.find(nick);
   cout << "nickIt: " << (nickIt != clientsByNick.end() ? nickIt->first : "not found") << endl;
    if (nickIt == clientsByNick.end())
        return NULL;

    map<int, Client>::const_iterator clientIt = clients.find(nickIt->second);
    if (clientIt == clients.end())
        return NULL;

    return &clientIt->second;
}

//  /connect localhost 1024 44