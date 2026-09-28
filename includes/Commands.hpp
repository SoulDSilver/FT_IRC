#pragma once

#include "irc.hpp"

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
        void TOPIC(const Channel& channel, const string& topic);
        void MODE(const Channel& channel, const string& mode);
        void JOIN(const Channel& channel, const Client& client);
        void INVITE(const Channel& channel, const Client& target);
        void PART(const Channel& channel, const Client& client);
        void PRIVMSG(const Client& sender, const Client& target, const string& message);
        void KICK(const Channel& channel, const Client& client, const string& reason);
        const string& getCommand() const { return command; }
        const string& getParameters() const { return parameters; }
        const string& getPrefix() const { return prefix; }
        static bool parse(const string &line, CommandList &commands);

};