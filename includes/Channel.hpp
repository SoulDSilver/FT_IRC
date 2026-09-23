#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "allincludes.hpp"
#include "Client.hpp"
#include "Server.hpp"

class Server;

class Channel
{
private:
    std::string name;
    Server *server;
    std::map<int , Client> clients;
    std::vector<std::string> operators;
    
public:
    Channel();
    Channel(const std::string &name, Server *server, Client &client);
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);
    ~Channel();
    const std::string &getName() const; 
    const  std::map<int , Client> &getClients() const;
    void addClient(const Client &client);
    void removeClient(const Client &client);
};

#endif