#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"

class Server;

class Channel
{
private:
    map<string, string> settings;
    Server &server;

    int Limit;
    bool InviteOnly;
    bool OnlyOperators;
    bool TopicRestricted;

    map<int, Client> clients;
    map<int, Client> operators;

    vector<int> invitedUsers;

public:
    Channel(const string &name, Server &server, const Client &client);
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);
    bool operator==(const Channel &other);
    ~Channel();

    const string &getName() const;
    const string &getPassword() const;
    const map<int, Client> &getClients() const;
    const map<int, Client> &getOperators() const;
    const map<string, string> &getSettings() const;
    const vector<int> &getInvitedUsers() const;

    void setPassword(const string &password);
    void addInvitedUser(int fd);
    void addOperator(const Client &client);
    void addClient(const Client &client);

    bool have_any_client() const;

    void removeClient(const Client &client);
    void removeOperator(const Client &client);
    void removeInvitedUser(const int &fd);
    void removeSetting(const string &setting);

    bool isClientPresent(int fd) const;
    bool isOperator(int fd) const;
    void addSetting(const string &setting, const string &value);
    void listclients() const;

    void sendJoinMessages(const Client &client) const;
    void sendPartMessage(const Client &client, const string &reason) const;
    void broadcastMessage(const string &message, int senderFd) const;
};

#endif
