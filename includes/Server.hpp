#pragma once

#include "irc.hpp"


#define MAXPENDCONN 10

class Client;
class Channel;

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
    std::map<int, Client> clients;
    std::map<std::string, Channel> channels;

	void createChannel(const std::string &channelName, Client &client);
    void addNewClient();
    void handleClientData(int client_fd);
    void removeClients(int client_fd);

  public:
	Server(int port, const std::string &password);
	Server(const Server &other);
	Server &operator=(const Server &other);
	~Server();

	void run();
    static void signalHandler(int signum);
    void closeFds(); 
 
	int getListenPort() const;
	int getListenFd() const;
};
