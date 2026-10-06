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
        static void processCommands(Server& server, const CommandList& parsedCommands, int fd)
        
        static void TOPIC(Server& server, const string& target, int fd);
        static void MODE( Channel& channel, const string& mode);
        static void JOIN(Server& server, const string& channelName, int fd);
        static void INVITE( Channel& channel, const Client& target);
        static void PART( Server& server, string& channelName, int fd);
        static void PRIVMSG(Server& server, const string& target, const string& message, int fd);
        static void KICK( Channel& channel, const Client& client, const string& reason);
        static void PING(Server& server, const string& serverName, int fd);

        static void pass(Server& server, const string& password, int fd);
        static void nick(Server& server, const string& nickname, int fd);
        static void user(Server& server, const string& username, int fd);
        static void join(Server& server, const string& channelName, int fd);
        static void part(Server& server, string& channelName, int fd);
        static void privmsg(Server& server, const string& target, const string& message, int fd);
        static void kick(Channel& channel, const Client& client, const string& reason);
        static void invite(Channel& channel, const Client& target);
        static void topic(Server& server, const string& target, int fd);
        static void mode(Channel& channel, const string& mode);
        static void quit(Server& server, const string& reason, int fd);
        static void ping(Server& server, const string& serverName, int fd);

};