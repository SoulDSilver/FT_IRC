#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "irc.hpp"

class Client
{
private:
	bool IsWelcome;
	bool HasHostname;

	int Fd;
	string Inbuff;
	string IpAddr;
	string Nick;
	string Username;
	string Hostname;
	string RealName;
	bool PasswordAccepted;
	bool HasNick;
	bool HasUsername;
	bool Registered;

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
	void setRegistered();
	bool isRegistered() const;
	void setHasUsername();
	bool hasUsername() const;
	void setHasNick();
	bool hasNick() const;
	void setWelcome();
	bool hasWelcome() const;
	const string &getInbuff() const;
	void appendInput(const string &inbuff);
	bool extractLine(string &line);
	void clearInBuff();
	bool isValid() const;
	void reset();
};

ostream &operator<<(std::ostream &stream, const Client &cl);
#endif