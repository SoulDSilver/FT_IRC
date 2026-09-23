#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"

class Server;


class Channel
{
private:
    string name;
    string password;
    Server &server;
    map<int , Client> clients;
    vector<int> operators;
    vector<string> settings;
    vector<string> invitedUsers;

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

    void addOperator(int fd);
    void removeOperator(int fd);
    bool isOperator(int fd) const;
    void addSetting(const string &setting);
    void removeSetting(const string &setting);
    void addInvitedUser(const string &username);
    void removeInvitedUser(const string &username);
    void setPassword(const string &password);
    void broadcastMessage(const string &message, int senderFd) const;

};

#endif
