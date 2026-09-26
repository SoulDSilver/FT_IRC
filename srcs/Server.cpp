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

void Server::removeClients(int fd, size_t index)
{
	this->Fds.erase(this->Fds.begin() + index);
	this->Clients.erase(fd);
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

void Server::addNewClient()
{
	struct sockaddr_in	cliadd;
	struct pollfd		NewPoll;
	socklen_t			len;
	int					incofd;

	Client cli;
	len = sizeof(cliadd);
	memset(&cliadd, 0, sizeof(cliadd));
	incofd = accept(this->Listen_fd, (sockaddr *)&(cliadd), &len);
	if (incofd == -1)
	{
		cout << "accept() failed" << endl;
		return ;
	}
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
	{
		cout << "fcntl() failed" << endl;
		close(incofd);
		return ;
	}

	NewPoll.fd = incofd;     //-> add the client socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0;     //-> set the revents to 0

	cli.setFd(incofd);
	cli.setIpAddr(inet_ntoa((cliadd.sin_addr)));
	this->Clients[incofd] = cli;
	this->Fds.push_back(NewPoll);

	cout << GRE << "Client <" << incofd << "> Connected" << WHI << endl;
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

void Server::handleClientData(int fd, size_t index)
{
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
	if (bytes <= 0)
	{
		cout << RED << "Client <" << fd << "> Disconnected" << WHI << endl;
		removeClients(fd, index);
		close(fd);
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
					handleClientData(Fds[i].fd, i);
			}
		}
	}
	cout << endl << "Signal Received!" << endl;
	cout << "The Server Closed!" << endl;
}
