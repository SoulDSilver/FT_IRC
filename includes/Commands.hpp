#pragma once

#include  "irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Client;
class Channel;

class Commands
{
    private:
        Commands();
        Commands(const Commands& other);
        Commands& operator=(const Commands& other);
        ~Commands();
        
    public:
        static void TOPIC(Channel& channel, const string& topic);
        static void MODE( Channel& channel, const string& mode);
        static void JOIN( Channel& channel, const Client& client);
        static void INVITE( Channel& channel, const Client& target);
        static void PART( Channel& channel, const Client& client);
        static void PRIVMSG(const map<string, Channel>& channels, const string& target, const string& message, int fd,  bool isClientInChannel);
        static void KICK( Channel& channel, const Client& client, const string& reason);
        static void PING( int fd, const string& serverName);
};