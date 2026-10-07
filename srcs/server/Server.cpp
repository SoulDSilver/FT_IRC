#include "Parser.hpp"
#include "Server.hpp"

Server::Server() : listen_port(0), password(""), listen_fd(-1)
{
}

Server::Server(int port, const string &password) : listen_port(port),
												   password(password), listen_fd(-1)
{
	name = "Broadcast_Server";
}

Server::Server(const Server &other) : listen_port(other.listen_port),
									  password(other.password), listen_fd(other.listen_fd), name(other.name)
{
}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		listen_port = other.listen_port;
		// password = other.password; // password is const, cannot be assigned
		listen_fd = other.listen_fd;
		name = other.name;
	}
	return (*this);
}

Server::~Server()
{
	closeFds(); //-> close all the clients and the server socket
}

void Server::closeFds()
{
	vector<struct pollfd>::iterator it;
	for (it = fds.begin(); it != fds.end(); ++it)
	{
		if (it->fd != -1 && it->fd != listen_fd)
		{
			cout << RED << "Client <" << it->fd << "> Disconnected" << WHI << endl;
			close(it->fd);
		}
	}
	if (this->listen_fd != -1)
	{ //-> close the server socket
		cout << RED << "Server <" << this->listen_fd << "> Disconnected" << WHI << endl;
		close(this->listen_fd);
	}
}

const string &Server::getName() const
{
	return (name);
}

const string &Server::getPassword() const
{
	return (password);
}

int Server::getListenPort() const
{
	return (listen_port);
}

int Server::getListenFd() const
{
	return (listen_fd);
}

volatile sig_atomic_t Server::Signal = 0;
void Server::signalHandler(int signum)
{
	(void)signum;
	Server::Signal = 1;
}

void Server::removeClients(int fd)
{
	for (vector<struct pollfd>::iterator it = this->fds.begin(); it != this->fds.end(); ++it)
	{
		if (it->fd == fd)
		{
			this->fds.erase(it);
			break;
		}
	}
	// Remove the client from every channel it belongs to.
	for (map<string,
			 Channel>::iterator it = this->channels.begin();
		 it != this->channels.end(); ++it)
	{

		if (it->second.isClientPresent(fd))
			it->second.removeClient(clients.at(fd));
	}

	close(fd);
	clientsByNick.erase(clients.at(fd).getNick());
	clients.erase(fd);
}

void Server::welcomeMessage(Client &client)
{
	string prefix;
	string burst;

	prefix = client.getNick() + "!" + client.getUsername() + "@" + client.getIpAddr();
	burst = ":" + name + " 001 " + client.getNick() + " :Welcome to the IRC Network " + prefix + "\r\n";
	burst += ":" + name + " 002 " + client.getNick() + " :Your host is " + name + ", running ircd-1.0\r\n";
	burst += ":" + name + " 003 " + client.getNick() + " :This server was created at the start of the session\r\n";
	burst += ":" + name + " 004 " + client.getNick() + " " + name + " ircd-1.0 o o\r\n";
	send(client.getFd(), burst.c_str(), burst.length(), 0);
}

void Server::pongmessage(int fd, const string &token)
{
	string reply;

	if (token.empty())
		return;
	reply = ":" + name + " PONG " + token + "\r\n";
	send(fd, reply.c_str(), reply.length(), 0);
}

void Server::handleCap(Client &client, const CommandPairVector &command)
{
	string subcommand;
	string reply;

	subcommand = Parser::getParam(command, PP_TARGET);
	if (subcommand == "LS" || subcommand == "NEW" || subcommand == "LIST")
	{
		reply = ":" + name + " CAP * LS :\r\n";
		send(client.getFd(), reply.c_str(), reply.length(), 0);
	}
	else if (subcommand == "REQ")
	{
		reply = ":" + name + " CAP * NAK :\r\n";
		send(client.getFd(), reply.c_str(), reply.length(), 0);
	}
}

bool Server::isClientInChannel(const string &channelName, int client_fd) const
{
	if (channels.count(channelName) == 0)
		return (false);
	return (channels.at(channelName).isClientPresent(client_fd));
}

bool Server::checkClientRegistered(Client &client)
{
	if (client.hasUsername() && client.hasNick() && client.isPasswordAccepted() && client.isRegistered() == false && client.hasWelcome() == false)
		return (true);
	return (false);
}

const Channel *Server::getChannel(const string &channelName) const
{
	if (channels.count(channelName) == 0)
		return (NULL);
	return (&channels.at(channelName));
}

const Client *Server::getClientByNick(const string &nick) const
{
	if (clientsByNick.empty())
	{
		cout << "clientsByNick is empty" << endl;
		return (NULL);
	}
	map<string, Client>::const_iterator it = clientsByNick.find(nick);
	if (it != clientsByNick.end())
		return (&it->second);
	return (NULL);
}

void Server::removeChannel(const string &channelname)
{
	channels.erase(channelname);
	cout << channelname << " have been erase!" << endl;
}

size_t Server::channelExists(const string &channelName)
{
	return (channels.count(channelName));
}

void Server::broadcastToChannel(const string &channelName,
								const string &message, const Client &sender)
{
	channels.at(channelName).broadcastMessage(message, sender);
}

void Server::addClientToChannel(const string &channelName, int fd)
{
	channels.at(channelName).addClient(clients.at(fd));
}

void Server::addClientToNickMap(const string &nick, const Client &client)
{
	clientsByNick.insert(make_pair(nick, client));
}


void Server::sendChannelJoinMessages(const string &channelName, int fd)
{
	channels.at(channelName).sendJoinMessages(clients.at(fd));
}

void Server::sendTopicMessages(const string &target, int fd)
{
	channels.at(target).sendMessage("Envia o Topico do canal se tiver", fd);
}

void Server::sendPartMessages(const string &channelName, int fd,
							  const string reason)
{
	channels.at(channelName).sendPartMessage(clients.at(fd), reason);
	channels.at(channelName).removeClient(clients.at(fd));
	cout << GRE << "Client <" << fd << "> removed from Channel <" << channelName << ">" << WHI << endl;
	if (channels.at(channelName).have_any_client())
		removeChannel(channelName);
}

void Server::create_socket()
{
	int yes;
	struct sockaddr_in addr;
	struct pollfd NewP;

	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(this->listen_port);
	addr.sin_family = AF_INET;
	this->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->listen_fd == -1)
		throw runtime_error("Error creating socket");
	yes = 1;
	if (setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
				   sizeof(yes)) == -1)
		throw(runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (bind(this->listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		throw(runtime_error("Error binding socket"));
	if (fcntl(this->listen_fd, F_SETFL, O_NONBLOCK) == -1)
		throw(runtime_error("Error setting socket to non-blocking"));
	if (listen(this->listen_fd, MAXPENDCONN) == -1)
		throw(runtime_error("Error listening on socket"));
	NewP.fd = this->listen_fd;
	NewP.events = POLLIN;
	NewP.revents = 0;
	fds.push_back(NewP);
}

void Server::createChannel(const string &channelName, Client &client)
{
	Channel channel(channelName, *this, client);
	channels.insert(make_pair(channelName, channel));
	cout << GRE << "Channel <" << channelName << "> Created by Client <" << client.getNick() << ">" << WHI << endl;
}

void Server::addNewClient()
{
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len;
	int incofd;
	Client cli;

	len = sizeof(cliadd);
	memset(&cliadd, 0, sizeof(cliadd));
	incofd = accept(this->listen_fd, (sockaddr *)&(cliadd), &len);
	if (incofd == -1)
	{
		cout << "accept() failed" << endl;
		return;
	}
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
	{
		cout << "fcntl() failed" << endl;
		close(incofd);
		return;
	}
	NewPoll.fd = incofd;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;
	cli.setFd(incofd);
	cli.setIpAddr(inet_ntoa((cliadd.sin_addr)));
	clients.insert(make_pair(incofd, cli));
	fds.push_back(NewPoll);
	cout << GRE << "Client <" << incofd << "> Connected" << WHI << endl;
}

void Server::handleClientData(int fd)
{
	char buff[1024];
	ssize_t bytes;
	string line;
	CommandList parsed;
	DispatchResult result;

	memset(buff, 0, sizeof(buff));
	bytes = recv(fd, buff, sizeof(buff) - 1, 0);
	if (bytes <= 0)
	{
		cout << RED << "Client <" << fd << "> Disconnected" << WHI << endl;
		removeClients(fd);
	}
	else
	{
		map<int, Client>::iterator client = this->clients.find(fd);
		if (client == this->clients.end())
			return;
		if (client->second.passfail == true)
		{
			cout << RED << "Client <" << fd << "> Disconnected due to password failure" << WHI << endl;
			removeClients(fd);
			return;
		}
		string data(buff);
		client->second.appendInput(data);
		cout << YEL << "Client <" << fd << "> Data: " << WHI;
		for (size_t i = 0; i < data.size(); i++)
		{
			if (data[i] == '\n')
				cout << "<LF>\n";
			else if (data[i] == '\r')
				cout << "<CR>";
			else
				cout << data[i];
		}
		while (client->second.extractLine(line))
		{
			if (!Parser::parse(line, parsed))
				continue;
			for (CommandList::iterator it = parsed.begin(); it != parsed.end(); ++it)
			{
				result = dispatchCommand(client->second, *it);
				if (result == DISPATCH_QUIT)
					return;
				if (result == DISPATCH_UNKNOWN)
				{
					sendNumericReply(client->second, "421", it->first + " :Unknown command");
				}
			}
		}
	}
}

void Server::run()
{
	create_socket();
	cout << GRE << "Server <" << this->listen_fd << "> Connected" << WHI << endl;
	cout << "Waiting to accept a connection...\n";
	while (Server::Signal == false)
	{
		if ((poll(fds.data(), fds.size(), -1) == -1) && Server::Signal == false)
			throw(runtime_error("poll() faild"));
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == this->listen_fd)
					addNewClient();
				else
					handleClientData(fds[i].fd);
			}
		}
	}
	cout << endl
		 << "Signal Received!" << endl;
	cout << "The Server Closed!" << endl;
}

void Server::actualizenicks(const string &oldNick, const string &newNick)
{
	if (clientsByNick.count(oldNick) == 0)
		return;
	Client &client = clientsByNick.at(oldNick);

	clients.at(client.getFd()).setNick(newNick);
	client.setNick(newNick);

}


Server::DispatchResult Server::dispatchCommand(Client &client,
											   const CommandPairVector &command)
{
	if (command.first == "PASS")
	{
		handlePass(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "NICK")
	{
		handleNick(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "USER")
	{
		handleUser(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "CAP")
	{
		handleCap(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "JOIN")
	{
		handleJoin(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "PART")
	{
		handlePart(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "PRIVMSG")
	{
		handlePrivmsg(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "MODE")
	{
		handleMode(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "WHOIS")
	{
		handleWhois(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "MOTD")
	{
		handleMotd(client, command);
		return (DISPATCH_OK);
	}
	if (command.first == "PING")
	{
		Commands::PING(*this, Parser::getParam(command, PP_TARGET),
					   client.getFd());
		return (DISPATCH_OK);
	}
	if (command.first == "QUIT")
	{
		handleQuit(client, command);
		return (DISPATCH_QUIT);
	}
	return (DISPATCH_UNKNOWN);
}


/*

/connect localhost 1024 44

*/