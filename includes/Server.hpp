#ifndef SERVER_HPP
#define SERVER_HPP

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

	void closeFds();
	void createChannel(const string &channelName, Client &client);
	void addNewClient();
	void handleClientData(int client_fd);
	void removeClients(int client_fd);
	void welcomeMessage(Client &client);
	void pongmessage(int fd, const string &a);
	bool isClientInChannel(const string &channelName, int client_fd) const;
	void removeChannel(const string &channelname);
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
	bool checkClientRegistered(Client &client);
	const string &getName() const;
	const string &getPassword() const;
	int getListenPort() const;
	int getListenFd() const;
};

#endif

