#include "irc.hpp"

class Client
{
private:
    int fd;
    std::string nickname;
    std::string username;

public:
    Client();
    Client(int fd, const std::string &password);
    Client(const Client &other);
    Client &operator=(const Client &other);
    ~Client();
    int getFd() const;
    const std::string &getPassword() const ;
};