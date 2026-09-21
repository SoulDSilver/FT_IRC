#include "Server.hpp"

Server::Server() : listen_port(0), password(""), listen_fd(-1)
{
}

Server::Server(int port, const std::string &password) : listen_port(port),
	password(password), listen_fd(-1)
{
}

Server::Server(const Server &other) : listen_port(other.listen_port),
	password(other.password), listen_fd(other.listen_fd)
{
}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		listen_port = other.listen_port;
		// password = other.password; // password is const, cannot be assigned
		listen_fd = other.listen_fd;
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
		std::cout << RED << "Client <" << clients[i].getFd() << "> Disconnected" << WHI << std::endl;
		close(clients[i].getFd());
	}
	if (this->listen_fd != -1)
	{ //-> close the server socket
		std::cout << RED << "Server <" << this->listen_fd << "> Disconnected" << WHI << std::endl;
		close(this->listen_fd);
	}
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
			break ;
		}
	}
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		if (this->clients[i].getFd() == fd)
		{
			this->clients.erase(this->clients.begin() + i);
			break ;
		}
	}
}

void Server::create_socket()
{
	int					yes;
	int					flags;
	struct sockaddr_in	addr;
	struct pollfd		NewP;

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(this->listen_port);
	addr.sin_family = AF_INET;
	//
	this->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->listen_fd == -1)
		throw std::runtime_error("Error creating socket");
	//
	yes = 1;
	if (setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(yes)) == -1)
	{
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	}
	//
	if (bind(this->listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		throw(std::runtime_error("Error binding socket"));
	}
	//
	flags = fcntl(this->listen_fd, F_GETFL, 0);
	if (flags == -1 || fcntl(this->listen_fd, F_SETFL, flags | O_NONBLOCK) ==
		-1)
	{
		throw(std::runtime_error("Error setting socket to non-blocking"));
	}
	//
	if (listen(this->listen_fd, MAXPENDCONN) == -1)
		throw(std::runtime_error("Error listening on socket"));
	//
	NewP.fd = this->listen_fd;
	NewP.events = POLLIN;
	NewP.revents = 0;
	fds.push_back(NewP);
}

void Server::addNewClient()
{
	struct sockaddr_in	cliadd;
	struct pollfd		NewPoll;
	socklen_t			len;
	int					incofd;
	int					flags;

	Client cli; //-> create a new client
	len = sizeof(cliadd);
	std::memset(&cliadd, 0, sizeof(cliadd));
	incofd = accept(this->listen_fd, (sockaddr *)&(cliadd), &len);
	if (incofd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		return ;
	}
	flags = fcntl(incofd, F_GETFL, 0);
	if (flags == -1 || fcntl(incofd, F_SETFL, flags | O_NONBLOCK) ==
		-1)
	{
		std::cout << "fcntl() failed" << std::endl;
		close(incofd);
		return ;
	}
	NewPoll.fd = incofd;     //-> add the client socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0;     //-> set the revents to 0
	cli.setFd(incofd);                          
		//-> set the client file descriptor
	cli.setIpAddr(inet_ntoa((cliadd.sin_addr)));
		//-> convert the ip address to string and set it
	clients.push_back(cli);                     
		//-> add the client to the vector of clients
	fds.push_back(NewPoll);                     
		//-> add the client socket to the pollfd
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::handleClientData(int fd)
{
	char buff[1024];               //-> buffer for the received data
	memset(buff, 0, sizeof(buff)); //-> clear the buffer
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0); //-> receive the data
	if (bytes <= 0)
	{ //-> check if the client disconnected
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		removeClients(fd); //-> clear the client
		close(fd);        //-> close the client socket
	}
	else
	{ //-> print the received data
		buff[bytes] = '\0';
		std::cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;
	}
}

void Server::run()
{
	create_socket();
	// std::cout << "accepting connections on port " << listen_port << std::endl;
	std::cout << GRE << "Server <" << this->listen_fd << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	while (Server::Signal == false)
	{ //-> run the server until the signal is received
		if ((poll(fds.data(), fds.size(), -1) == -1) && Server::Signal == false)
			throw(std::runtime_error("poll() faild"));
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
					handleClientData(fds[i].fd); //-> handle data from existing client
				}
			}
		}
	}
	std::cout << std::endl << "Signal Received!" << std::endl;
	std::cout << "The Server Closed!" << std::endl;
}
