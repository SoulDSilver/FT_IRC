#pragma once

#include "irc.hpp"
#include "Client.hpp"
#include "Server.hpp"

class Server;
class Client;

class Channel
{
private:
    string Name;
    Server &ServerRef;
    map<int , Client> Clients;
    vector<int> Operators;
    vector<string> Settings;
    vector<string> InvestedUsers;

public:
    Channel(const string &name, Server &server, Client &client);
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);
    bool operator==(const Channel &other);
    ~Channel();
    const string &getName() const; 
    const  map<int , Client> &getClients() const;
    void addClient(const Client &client);
    void removeClient(const Client &client);
};

