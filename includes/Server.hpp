#pragma once

#include "irc.hpp"
#include "Client.hpp"

#define MAXPENDCONN 10

class Server
{
  private:
	int listen_port;
	const std::string password;
	int listen_fd;
	static volatile sig_atomic_t Signal;
	Server();
	void create_socket();
	std::vector<struct pollfd> fds;
    std::vector<Client> clients;
    void addNewClient();
    void handleClientData(int client_fd);
    void removeClients(int client_fd);

  public:
	Server(int port, const std::string &password);
	Server(const Server &other);
	Server &operator=(const Server &other);
	~Server();

	// importants methods
  public:
	void run();
    static void signalHandler(int signum); //-> static method to handle signals
    void closeFds(); //-> method to close all the clients and the server socket

	// geters and setters
  public:
	int getListenPort() const;
	int getListenFd() const;
};
