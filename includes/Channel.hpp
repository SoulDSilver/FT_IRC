#pragma once

#include "irc.hpp"
#include "Client.hpp"
#include "Server.hpp"

class Server;
class Client;

class Channel
{
private:
    std::string name;
    Server &server;
    std::map<int , Client> clients;
    std::vector<std::string> operators;


public:
    Channel(const std::string &name, Server &server, Client &client);
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);
    bool operator==(const Channel &other);
    ~Channel();
    const string &getName() const; 
    const  map<int , Client> &getClients() const;
    void addClient(const Client &client);
    void removeClient(const Client &client);
};

