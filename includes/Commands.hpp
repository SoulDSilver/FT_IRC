#pragma once

#include  "irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Client;
class Channel;

class Commands
{
    private:
        string command;
        string parameters;
        string prefix;

    public:
        Commands();
        Commands(const string& command, const string& parameters, const string& prefix)
            : command(command), parameters(parameters), prefix(prefix) {}
        Commands(const Commands& other);
        Commands& operator=(const Commands& other);
        ~Commands();
        void TOPIC(Channel& channel, const string& topic);
        void MODE( Channel& channel, const string& mode);
        void JOIN( Channel& channel, const Client& client);
        void INVITE( Channel& channel, const Client& target);
        void PART( Channel& channel, const Client& client);
        void PRIVMSG(const Client& sender, const Client& target, const string& message);
        void KICK( Channel& channel, const Client& client, const string& reason);
        const string& getCommand() const { return command; }
        const string& getParameters() const { return parameters; }
        const string& getPrefix() const { return prefix; }
};