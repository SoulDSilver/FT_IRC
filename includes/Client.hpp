#include "irc.hpp"

class Client
{
  private:
	int fd;
	std::string ipAddr;
	std::string username;

  public:
	Client();
	Client(const Client &other);
	Client &operator=(const Client &other);
	~Client();

	int getFd() const;
    void setFd(int fd);
    const std::string &getIpAddr() const;
    void setIpAddr(const std::string &ipAddr);
	const std::string &getUsername() const;
    void setUsername(const std::string &username);
};