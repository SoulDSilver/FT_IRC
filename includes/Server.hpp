#ifndef SERVER_HPP
#define SERVER_HPP

#include "allincludes.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#define MAXPENDCONN 10

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

	// importants methods
	void run();
    static void signalHandler(int signum); //-> static method to handle signals
    void closeFds(); //-> method to close all the clients and the server socket

	// geters and setters
 
	int getListenPort() const;
	int getListenFd() const;
};

#endif
