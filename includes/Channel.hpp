#pragma once

#include "irc.hpp"
#include "Client.hpp"


class Channel
{
private:
    std::string name;
    std::vector<Client> clients;

public:
    Channel();
    Channel(const std::string &name);
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);
    ~Channel();
    const std::string &getName() const; 
    const std::vector<Client> &getClients() const;
    void addClient(const Client &client);
    void removeClient(const Client &client);
};