#pragma once


#include "Client.hpp"
#include "Channel.hpp"

#define MAXPENDCONN 10

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


	void createChannel(const string &channelName, Client &client);
    void addNewClient();
    void handleClientData(int client_fd);
    void removeClients(int client_fd);

  public:
	Server(int port, const string &password);
	Server(const Server &other);
	Server &operator=(const Server &other);
	~Server();

	// importants methods
	void run();
    static void signalHandler(int signum); //-> static method to handle signals
    void closeFds(); //-> method to close all the clients and the server socket

	// geters and setters
 
	int getListenPort() const;
	int getListenFd() const;
};
