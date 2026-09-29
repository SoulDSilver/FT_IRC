#pragma once

#include "irc.hpp"

class Client
{
  private:
	int Fd;
	string Inbuff;
	string IpAddr;
	string Nick;
	string Username;
	string RealName;
	bool PasswordAccepted;
	bool HasNick;
	bool HasUsername;
	
public:
	Client();
	Client(const Client &other);
	Client &operator=(const Client &other);
	~Client();

	int getFd() const;
    void setFd(int fd);
    const string &getIpAddr() const;
    void setIpAddr(const string &ipAddr);
	const string &getUsername() const;
	string getNick() const;
	void setNick(const string &nick);
    void setUsername(const string &username);
    void setRealName(const string &realName);
    const string &getRealName() const;
    void setHasUsername();
    bool hasUsername() const;
    void setPasswordAccepted();
    bool isPasswordAccepted() const;
    void setHasNick();
    bool hasNick() const;
    void appendInput(const string &data);
    const string &getInbuff() const;
    bool extractLine(string &line);
};

