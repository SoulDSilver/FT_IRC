#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include "Commands.hpp"

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
	map<string, Client> clientsByNick;
	map<string, Channel> channels;

	void closeFds();
	void addNewClient();
	void handleClientData(int client_fd);
	void removeClients(int client_fd);
	void handleCap(Client &client, const CommandPairVector &command);
	enum DispatchResult
	{
		DISPATCH_OK,
		DISPATCH_QUIT,
		DISPATCH_UNKNOWN
	};
	DispatchResult dispatchCommand(Client &client, const CommandPairVector &command);
	void handlePass(Client &client, const CommandPairVector &command);
	void handleNick(Client &client, const CommandPairVector &command);
	void handleUser(Client &client, const CommandPairVector &command);
	void handleQuit(Client &client, const CommandPairVector &command);
	void handleJoin(Client &client, const CommandPairVector &command);
	void handlePart(Client &client, const CommandPairVector &command);
	void handlePrivmsg(Client &client, const CommandPairVector &command);
	void handleMode(Client &client, const CommandPairVector &command);
	void handleWhois(Client &client, const CommandPairVector &command);
	void handleMotd(Client &client, const CommandPairVector &command);

public:
	Server(int port, const string &password);
	Server(const Server &other);
	Server &operator=(const Server &other);
	~Server();

	void run();
	static void signalHandler(int signum);
	const string &getName() const;
	const string &getPassword() const;
	int getListenPort() const;
	int getListenFd() const;

	// Operations used by the Commands interface.
	void pongmessage(int fd, const string &token);
	void addClientToChannel(const string &channelName, int fd);
	void addClientToNickMap(const string &nick, const Client &client);
	size_t channelExists(const string &channelName);
	bool isClientInChannel(const string &channelName, int client_fd) const;
	void broadcastToChannel(const string &channelName, const string &message, const Client &sender);
	void sendTopicMessages(const string &target, int fd);
	void sendPartMessages(const string &channelName, int fd, const string reason);
	void createChannel(const string &channelName, Client &client);
	void sendChannelJoinMessages(const string &channelName, int fd);

	void sendNumericReply(Client &client, const string &code, const string &parameters);
	void welcomeMessage(Client &client);

	void removeChannel(const string &channelname);

	bool checkClientRegistered(Client &client);

	const Channel *getChannel(const string &channelName) const ;
	const Client *getClientByNick(const string &nick) const;
};

#endif
