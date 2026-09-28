#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "irc.hpp"

class Client
{
private:
	int fd;
	string inbuff;
	string outbuff;
	string ipAddr;
	string nick;
	string username;
	string realName;
	bool passwordAccepted;
	bool hasNick;
	bool hasUsername;

public:
	Client();
	Client(const Client &other);
	Client &operator=(const Client &other);
	bool operator==(const Client &other) const;

	~Client();

	int getFd() const;
	void setFd(int fd);
	const string &getIpAddr() const;
	void setIpAddr(const string &ipAddr);
	const string &getUsername() const;
	string getNick() const;
	void setNick(const string &nick);
	void setUsername(const string &username);
	const string &getRealName() const;
	void setRealName(const string &realName);
	bool isPasswordAccepted() const;
	void setPasswordAccepted();
	bool hasNickSet() const;
	void setHasNick();
	bool hasUsernameSet() const;
	void setHasUsername();

	const string &getInbuff() const;
	void appendInput(const string &inbuff);
	const string &getOutBuff() const;
	bool extractLine(string &line);
	void setOutBuff(const string &outbuff);
	void appendToOutBuff(const string &data);
	void clearInBuff();
	void clearOutBuff();
	bool isValid() const;
	void reset();
};

ostream &operator<<(std::ostream &stream, const Client &cl);
#endif