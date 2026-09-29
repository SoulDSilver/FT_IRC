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
	int Listen_port;
	const string Password;
	int Listen_fd;
	static volatile sig_atomic_t Signal;
	string Name;
	Server();
	void create_socket();

	vector<struct pollfd> Fds;
    map<int, Client> Clients;
    map<string, Channel> Channels;

	void createChannel(const std::string &channelName, Client &client);
    void addNewClient();
    void handleClientData(int client_fd);
    void removeClients(int client_fd);
    bool dispatchCommand(Client &client, const CommandPairVector &command);
    void handlePass(Client &client, const CommandPairVector &command);
    void handleNick(Client &client, const CommandPairVector &command);
    void handleUser(Client &client, const CommandPairVector &command);
    void handleQuit(Client &client, const CommandPairVector &command);
    void sendNumericReply(Client &client, const string &code, const string &parameters);

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
	bool checkClientRegistered(Client &client);
};
