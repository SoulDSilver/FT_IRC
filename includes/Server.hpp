#pragma once

#include "irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#define MAXPENDCONN 10

class Client;
class Channel;

class Server
{
  private:
	int listen_port;
	const string password;
	int listen_fd;
	static volatile sig_atomic_t Signal;
	string name;
	Server();
	void create_socket();

	vector<struct pollfd> fds;
    map<int, Client> clients;
    map<string, Channel> channels;

	void createChannel(const std::string &channelName, Client &client);
    void addNewClient();
    void handleClientData(int client_fd);
    void removeClients(int client_fd);

  public:
	Server(int port, const string &password);
	Server(const Server &other);
	Server &operator=(const Server &other);
	~Server();

	void run();
    static void signalHandler(int signum);
    void closeFds(); 
 
	int getListenPort() const;
	int getListenFd() const;
};
