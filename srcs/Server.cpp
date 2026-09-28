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
	for (size_t i = 0; i < clients.size(); i++)
	{ //-> close all the clients
		cout << RED << "Client <" << clients[i].getFd() << "> Disconnected" << WHI << endl;
		close(clients[i].getFd());
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
	for (size_t i = 0; i < this->fds.size(); i++)
	{
		if (this->fds[i].fd == fd)
		{
			this->fds.erase(this->fds.begin() + i);
			break;
		}
	}
	clients.erase(fd);
}

void Server::welcomeMessage(Client &client)
{
	string welcomeMsg;

	welcomeMsg = ":" + this->getName() + " 001 " + client.getNick() + " :Welcome to the IRC server, " + client.getNick() + "!\r\n";
	send(client.getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
}

void Server::pongmessage(int fd, const vector<string> &token)
{
	string retorno;

	if (token.size() == 2 && token[0] == "PING")
	{
		retorno = "PONG " + token[1] + "\r\n";
		send(fd, retorno.c_str(), retorno.size(), 0);
	}
}

bool Server::isClientInChannel(const string &channelName, int client_fd) const
{
	if (channels.count(channelName) == 1)
	{
		const map<int,
				  Client> &channelClients = channels.at(channelName).getClients();
		return (channelClients.find(client_fd) != channelClients.end());
	}
	return (false);
}

void Server::removeChannel(const string &channelname)
{
	channels.erase(channelname);
	cout << channelname << " have been erase!" << endl;
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
	//
	this->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->listen_fd == -1)
		throw runtime_error("Error creating socket");
	//
	yes = 1;
	if (setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
				   sizeof(yes)) == -1)
	{
		throw(runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	}
	//
	if (bind(this->listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		throw(runtime_error("Error binding socket"));
	}
	//
	if (fcntl(this->listen_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw(runtime_error("Error setting socket to non-blocking"));
	}
	//
	if (listen(this->listen_fd, MAXPENDCONN) == -1)
		throw(runtime_error("Error listening on socket"));
	//
	NewP.fd = this->listen_fd;
	NewP.events = POLLIN;
	NewP.revents = 0;
	fds.push_back(NewP);
}

void Server::createChannel(const string &channelname, Client &client)
{
	Channel channel(channelname, *this, client);
	channels.insert(make_pair(channelname, channel));
	cout << GRE << "Channel <" << channelname << "> Created by Client <" << client.getNick() << ">" << WHI << endl;
}

void Server::addNewClient()
{
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len;
	int incofd;
	string jj;

	Client cli; //-> create a new client
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
	NewPoll.fd = incofd;	 //-> add the client socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0;	 //-> set the revents to 0
	cli.setFd(incofd);
	//-> set the client file descriptor
	cli.setIpAddr(inet_ntoa((cliadd.sin_addr)));
	//-> convert the ip address to string and set it
	clients[incofd] = cli;
	//-> add the client to the map keyed by file descriptor
	fds.push_back(NewPoll);
	std::ostringstream guestName;
	guestName << "Guest" << incofd;
	clients.at(incofd).setUsername(guestName.str());
	clients.at(incofd).setNick(guestName.str());
	welcomeMessage(clients.at(incofd));
	//-> add the client socket to the pollfd
	cout << GRE << "Client <" << incofd << "> Connected" << WHI << endl;
}

void Server::handleClientData(int fd)
{
	char buff[1024];
	string channelName;
	size_t reasonPosition;
	string reason;
	size_t pos;
	string target;
	string message;
	string serverName;

	//-> buffer for the received data
	memset(buff, 0, sizeof(buff));						 //-> clear the buffer
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0); //-> receive the data
	if (bytes <= 0)
	{ //-> check if the client disconnected
		cout << RED << "Client <" << fd << "> Disconnected" << WHI << endl;
		removeClients(fd); //-> clear the client
		close(fd);		   //-> close the client socket
	}
	else
	{ //-> print the received data
		// parser parte
		string data(buff);
		/*meu minni parser*/
		vector<string> tokens;
		if (data.compare(0, 4, "PING") == 0)
		{
			serverName = data.substr(4);
			serverName.erase(remove(serverName.begin(), serverName.end(), '\r'),
							 serverName.end());
			serverName.erase(remove(serverName.begin(), serverName.end(), '\n'),
							 serverName.end());
			if (!serverName.empty())
			{
				tokens.push_back("PING");
				tokens.push_back(serverName);
				pongmessage(fd, tokens);
			}
		}
		/*================*/
		if (data.substr(0, 4) == "JOIN")
		{
			channelName = data.substr(5);
			channelName.erase(remove(channelName.begin(), channelName.end(),
									 '\r'),
							  channelName.end());
			channelName.erase(remove(channelName.begin(), channelName.end(),
									 '\n'),
							  channelName.end());
			if (channels.count(channelName) == 0)
			{
				createChannel(channelName, clients.at(fd));
				channels.at(channelName).sendJoinMessages(clients.at(fd));
			}
			else if (channels.count(channelName) == 1)
			{
				channels.at(channelName).addClient(clients.at(fd));
				channels.at(channelName).sendJoinMessages(clients.at(fd));
				cout << GRE << "Client <" << fd << "> Joined Channel <" << channelName << ">" << WHI << endl;
			}
		}
		else if (data.substr(0, 4) == "PART")
		{
			channelName = data.substr(5);
			channelName.erase(remove(channelName.begin(), channelName.end(),
									 '\r'),
							  channelName.end());
			channelName.erase(remove(channelName.begin(), channelName.end(),
									 '\n'),
							  channelName.end());
			reasonPosition = channelName.find(" :");
			if (reasonPosition != string::npos)
			{
				reason = channelName.substr(reasonPosition + 2);
				channelName = channelName.substr(0, reasonPosition);
			}
			if (channels.count(channelName) == 1 && isClientInChannel(channelName, fd))
			{
				channels.at(channelName).sendPartMessage(clients.at(fd), reason);
				channels.at(channelName).removeClient(clients.at(fd));
				cout << GRE << "Client <" << fd << "> removed from Channel <" << channelName << ">" << WHI << endl;
				if (channels.at(channelName).have_any_client())
					removeChannel(channelName);
			}
		}
		if (data.substr(0, 7) == "PRIVMSG")
		{
			pos = data.find(" ");
			if (pos != string::npos)
			{
				target = data.substr(pos + 1, data.find(" ", pos + 1) - pos - 1);
				cout << GRE << "target" << target << WHI << endl;
				message = data.substr(data.find(":", pos) + 1);
				message.erase(remove(message.begin(), message.end(), '\r'),
							  message.end());
				message.erase(remove(message.begin(), message.end(), '\n'),
							  message.end());
				if (channels.count(target) == 1)
				{
					if (isClientInChannel(target, fd))
						channels.at(target).broadcastMessage(message, fd);
				}
			}
		}
		buff[bytes] = '\0';
		cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;
	}
}

void Server::run()
{
	create_socket();
	// cout << "accepting connections on port " << listen_port << endl;
	cout << GRE << "Server <" << this->listen_fd << "> Connected" << WHI << endl;
	cout << "Waiting to accept a connection...\n";
	while (Server::Signal == false)
	{ //-> run the server until the signal is received
		if ((poll(fds.data(), fds.size(), -1) == -1) && Server::Signal == false)
			throw(runtime_error("poll() faild"));
		for (size_t i = 0; i < fds.size(); i++)
		{ //-> check all file descriptors
			if (fds[i].revents & POLLIN)
			{ //-> check if there is data to read revents = POLLIN
				if (fds[i].fd == this->listen_fd)
				{
					addNewClient(); //-> accept new client
				}
				else
				{
					handleClientData(fds[i].fd);
					//-> handle data from existing client
				}
			}
		}
	}
	cout << endl
		 << "Signal Received!" << endl;
	cout << "The Server Closed!" << endl;
}


#include "Server.hpp"
#include "Commands.hpp"

Server::Server() : Listen_port(0), Password(""), Listen_fd(-1)
{
}

Server::Server(int port, const string &password) : Listen_port(port),
	Password(password), Listen_fd(-1)
{
	Name = "Broadcast_Server";
}

Server::Server(const Server &other) : Listen_port(other.Listen_port),
	Password(other.Password), Listen_fd(other.Listen_fd), Name(other.Name)
{
}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		this->Listen_port = other.Listen_port;
		// password = other.password; // password is const, cannot be assigned
		this->Listen_fd = other.Listen_fd;
		this->Name = other.Name;
	}
	return (*this);
}

Server::~Server()
{
	closeFds(); //-> close all the clients and the server socket
}

void Server::closeFds()
{
	for (size_t i = 0; i < this->Clients.size(); i++)
	{ //-> close all the clients
		cout << RED << "Client <" << this->Clients[i].getFd() << "> Disconnected" << WHI << endl;
		close(this->Clients[i].getFd());
	}
	if (this->Listen_fd != -1)
	{ //-> close the server socket
		cout << RED << "Server <" << this->Listen_fd << "> Disconnected" << WHI << endl;
		close(this->Listen_fd);
	}
}

int Server::getListenPort() const
{
	return (this->Listen_port);
}

int Server::getListenFd() const
{
	return (this->Listen_fd);
}

volatile sig_atomic_t Server::Signal = 0;
void Server::signalHandler(int signum)
{
	(void)signum;
	Server::Signal = 1;
}

void Server::removeClients(int fd)
{
	if (fd == this->Listen_fd)
		return;

	map<int, Client>::iterator client = this->Clients.find(fd);
	if (client == this->Clients.end())
		return;

	for (map<string, Channel>::iterator it = this->Channels.begin();
		it != this->Channels.end(); ++it)
		it->second.removeClient(client->second);

	map<string, Channel>::iterator channel = this->Channels.begin();
	while (channel != this->Channels.end())
	{
		if (channel->second.getClients().empty())
		{
			map<string, Channel>::iterator empty = channel++;
			this->Channels.erase(empty);
		}
		else
			++channel;
	}

	for (vector<struct pollfd>::iterator it = this->Fds.begin();
		it != this->Fds.end(); ++it)
	{
		if (it->fd == fd)
		{
			this->Fds.erase(it);
			break;
		}
	}

	cout << RED << "Client <" << fd << "> Disconnected" << WHI << endl;
	close(fd);
	this->Clients.erase(client);
}

void Server::create_socket()
{
	int					yes;
	struct sockaddr_in	addr;
	struct pollfd		NewP;

	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(this->Listen_port);
	addr.sin_family = AF_INET;

	this->Listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->Listen_fd == -1)
		throw runtime_error("Error creating socket");
	
	yes = 1;
	if (setsockopt(this->Listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(yes)) == -1)
		throw(runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (bind(this->Listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		throw(runtime_error("Error binding socket"));
	if (fcntl(this->Listen_fd, F_SETFL, O_NONBLOCK) == -1)
		throw(runtime_error("Error setting socket to non-blocking"));
	if (listen(this->Listen_fd, MAXPENDCONN) == -1)
		throw(runtime_error("Error listening on socket"));

	NewP.fd = this->Listen_fd;
	NewP.events = POLLIN;
	NewP.revents = 0;
	this->Fds.push_back(NewP);
}

void Server::createChannel(const string &channelname, Client &client)
{
	Channel channel(channelname, *this, client);
	this->Channels.insert(make_pair(channelname, channel));
}



bool Server::dispatchCommand(Client &client, const pair<string, string> &command)
{
	if (command.first == "PASS")
	{
		handlePass(client, command.second);
		return (true);
	}
	if (command.first == "NICK")
	{
		handleNick(client, command.second);
		return (true);
	}
	if (command.first == "USER")
	{
		handleUser(client, command.second);
		return (true);
	}
	if (command.first == "QUIT")
	{
		handleQuit(client, command.second);
		return (false);
	}
	return (false);
}

void Server::handleClientData(int fd)
{
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
	if (bytes <= 0)
	{
		removeClients(fd);
		return;
	}

	map<int, Client>::iterator client = this->Clients.find(fd);
	if (client == this->Clients.end())
		return;

	string data(buff, static_cast<size_t>(bytes));
	client->second.appendInput(data);
	cout << YEL << "Client <" << fd << "> Data: " << WHI << data << endl;

	string line;
	while (client->second.extractLine(line))
	{
		CommandList parsed;
		if (!Commands::parse(line, parsed))
			continue;
		for (CommandList::iterator it = parsed.begin();
			it != parsed.end(); ++it)
		{
			if (!dispatchCommand(client->second, *it))
				return;
		}
	}
}

void Server::run()
{
	create_socket();
	cout << GRE << "Server <" << this->Listen_fd << "> Connected" << WHI << endl;
	cout << "Waiting to accept a connection...\n";
	while (Server::Signal == false)
	{ //-> run the server until the signal is received
		if ((poll(Fds.data(), Fds.size(), -1) == -1) && Server::Signal == false)
			throw(runtime_error("poll() faild"));
		for (size_t i = 0; i < Fds.size(); i++)
		{
			if (Fds[i].revents & POLLIN)
			{
				if (Fds[i].fd == this->Listen_fd)
					addNewClient(); //-> accept new client
				else
					handleClientData(Fds[i].fd);
			}
		}
	}
	cout << endl << "Signal Received!" << endl;
	cout << "The Server Closed!" << endl;
}
