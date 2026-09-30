#ifndef SERVER_HPP
#define SERVER_HPP


#include "Client.hpp"
#include "Channel.hpp"
#include "Commands.hpp"

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
	void welcomeMessage(Client &client);
	bool isClientInChannel(const string &channelName, int client_fd) const;
	void pongmessage(int fd, const string &a);
	void removeChannel(const string &channelname);

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
 
	const string &getName() const;
	const string &getPassword() const;
	int getListenPort() const;
	int getListenFd() const;
};

#endif
