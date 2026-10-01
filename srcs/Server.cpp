#include "Server.hpp"
#include "Commands.hpp"

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
	for (vector<struct pollfd>::iterator it = this->fds.begin();
		 it != this->fds.end(); ++it)
	{
		if (it->fd == fd)
		{
			this->fds.erase(it);
			break;
		}
	}
	// Remove the client from every channel it belongs to.
	for (map<string, Channel>::iterator it = this->channels.begin(); it != this->channels.end(); ++it)
		if (it->second.isClientPresent(fd))
			it->second.removeClient(clients.at(fd));
	close(fd);
	clients.erase(fd);
}

void Server::welcomeMessage(Client &client)
{
	string welcomeMsg;

	welcomeMsg = ":" + this->getName() + " 001 " + client.getNick() + " :Welcome to the IRC server, " + client.getNick() + "!\r\n";
	send(client.getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	client.setWelcome();
}

void Server::pongmessage(int fd, const string &a)
{
	string retorno;

	if (!a.empty())
	{
		retorno = "PONG " + a + "\r\n";
		send(fd, retorno.c_str(), retorno.size(), 0);
	}
	else
		send(fd, "PONG :ola do server \r\n", 23, 0);
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

bool Server::checkClientRegistered(Client &client)
{
        if (client.hasUsername() && client.hasNick() && client.isPasswordAccepted() && client.isRegistered() == false)
                return true;
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
	string channelName;
	size_t reasonPosition;
	string reason;
	size_t pos;
	string target;
	string message;
	string serverName;

	memset(buff, 0, sizeof(buff));
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);

	if (bytes <= 0)
	{
		cout << RED << "Client <" << fd << "> Disconnected" << WHI << endl;
		removeClients(fd);
		close(fd);
	}
	else
	{
		map<int, Client>::iterator client = this->clients.find(fd);
		if (client == this->clients.end())
			return;
		if(!client->second.hasWelcome())
			welcomeMessage(client->second);
		string data(buff);
		if (data.compare(0, 4, "PING") == 0)
		{
			serverName = data.substr(4);
			serverName.erase(remove(serverName.begin(), serverName.end(), '\r'),
							 serverName.end());
			serverName.erase(remove(serverName.begin(), serverName.end(), '\n'),
							 serverName.end());
			if (!serverName.empty())
				pongmessage(fd, serverName);
		}
		string cca(buff);
		string line;

		client->second.appendInput(cca);
		cout << YEL << "Client <" << fd << "> Data: " << WHI << cca << endl;

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


		/*================================================================================*/
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
		/*================================================================================*/
		if (data.substr(0, 4) == "PART")
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
		/*================================================================================*/
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

// typedef pair<int, string> CommandPair;
// typedef pair<string, vector<CommandPair> > CommandPairVector;
// typedef vector<CommandPairVector> CommandList;
//

bool Server::dispatchCommand(Client &client, const CommandPairVector &command)
{
        if (command.first == "PASS")
        {
                handlePass(client, command);
                return (true);
        }
        if (command.first == "NICK")
        {
                handleNick(client, command);
                return (true);
        }
        if (command.first == "USER")
        {
                handleUser(client, command);
                return (true);
        }
        if (command.first == "QUIT")
        {
                handleQuit(client, command);
                return (false);
        }
        return (false);
}

