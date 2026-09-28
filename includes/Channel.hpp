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
    map<int, Client> clients;
    map<int, Client> operators;
    vector<string> settings;
    vector<string> invitedUsers;

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
    const vector<string> &getSettings() const;
    const vector<string> &getInvitedUsers() const;

    bool  have_any_client() const;
    void addClient(const Client &client);
    void removeClient(const Client &client);

    void listclients() const;
    void addOperator(const Client &client);
    void removeOperator(const Client &client);
    bool isOperator(int fd) const;
    void addSetting(const string &setting);
    void removeSetting(const string &setting);
    void addInvitedUser(const string &username);
    void removeInvitedUser(const string &username);
    void setPassword(const string &password);
    void sendJoinMessages(const Client &client) const;
    void sendPartMessage(const Client &client, const string &reason) const;
    void broadcastMessage(const string &message, int senderFd) const;
};

#endif
