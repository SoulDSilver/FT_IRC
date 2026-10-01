#pragma once

#include  "irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

class Client;
class Channel;
class Server;

class Commands
{
    private:
        Commands();
        Commands(const Commands& other);
        Commands& operator=(const Commands& other);
        ~Commands();
        
    public:
        static void TOPIC(Server& server, const string& target, int fd);
        static void MODE( Channel& channel, const string& mode);
        static void JOIN(Server& server, const string& channelName, int fd);
        static void INVITE( Channel& channel, const Client& target);
        static void PART( Server& server, string& channelName, int fd);
        static void PRIVMSG(Server& server, const string& target, const string& message, int fd);
        static void KICK( Channel& channel, const Client& client, const string& reason);
        static void PING(Server& server, const string& serverName, int fd);
};