#pragma once

#include "irc.hpp"

class Client
{
  private:
	int fd;
	string ipAddr;
	string username;

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
    void setUsername(const string &username);
};

