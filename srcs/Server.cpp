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
	string welcomeMsg = ":" + this->getName() + " 001 " + client.getNick() + " :Welcome to the IRC server, " + client.getNick() + "!\r\n";
	send(client.getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
}

bool Server::isClientInChannel(const string &channelName, int client_fd) const
{
	if (channels.count(channelName) == 1)
	{
		const map<int, Client> &channelClients = channels.at(channelName).getClients();
		return channelClients.find(client_fd) != channelClients.end();
	}
	return false;
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
		vector<string> dt;
		size_t start = 0;
		size_t pos = data.find(' ', start);
		while(pos != string::npos){
			dt.push_back(data.substr(start, pos - start));
			start = pos + 1;
			pos = data.find(' ', start);
		}
		dt.push_back(data.substr(start));
		dt.back().erase(remove(dt.back().begin(), dt.back().end(), '\r'), dt.back().end());
		dt.back().erase(remove(dt.back().begin(), dt.back().end(), '\n'), dt.back().end());
		
		for (size_t i = 0; i < dt.size(); i++){
    		cout << i << " - " << dt[i] << " size: " << dt[i].size() << endl;
		}
		string command = dt[0];
		for (size_t i = 0; i < command.size(); i++)
    		command[i] = toupper(command[i]);

		if (command == "JOIN")
		{
			string channelName = dt[1];
			channelName.erase(remove(channelName.begin(), channelName.end(), '\r'), channelName.end());
			channelName.erase(remove(channelName.begin(), channelName.end(), '\n'), channelName.end());
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
		else if (command == "PART")
		{
			string channelName = dt[1];
			channelName.erase(remove(channelName.begin(), channelName.end(), '\r'), channelName.end());
			channelName.erase(remove(channelName.begin(), channelName.end(), '\n'), channelName.end());
			size_t reasonPosition = channelName.find(" :");
			string reason;
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
		if (command == "PRIVMSG")
		{
			
				string target = dt[1];
				cout << GRE << "target" << target << WHI << endl;
				string message = dt[2];
				message.erase(remove(message.begin(), message.end(), '\r'), message.end());
				message.erase(remove(message.begin(), message.end(), '\n'), message.end());
				if (channels.count(target) == 1)
				{
					if (isClientInChannel(target, fd))
						channels.at(target).broadcastMessage(message, fd);
				}
			
		}
		buff[bytes] = '\0';
		cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;
		//verificando se esta vazio

		if(command == "TOPIC"){
			if(dt.size() == 2){
				string target = dt[1];
				if(channels.count(target) == 1){
					cout << "canal: " << dt[1] << endl;
					channels.at(target).sendMessge("Nome do Canal se tiver", fd);
				}
			}
		}

		if()

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
