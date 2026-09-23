#pragma once

#include "irc.hpp"
#include "Client.hpp"
#include "Server.hpp"

class Server;
class Client;

class Channel
{
private:
    string name;
    Server &server;
    map<int , Client> clients;
    vector<int> operators;
    vector<string> settings;
    vector<string> investedUsers;

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

