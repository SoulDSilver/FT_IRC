#pragma once

#include "irc.hpp"

class Client
{
  private:
	int Fd;
	string Inbuff;
	string Outbuff;
	string IpAddr;
	string Nick;
	string Username;
	
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
    void appendInput(const string &data);
    const string &getInbuff() const;
    bool extractLine(string &line);
};

