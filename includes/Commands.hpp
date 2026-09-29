#pragma once

#include  "irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Client;
class Channel;

class Commands
{
    private:
        string Command;
        string Parameters;
        string Prefix;

    public:
        Commands();
        Commands(const string& command, const string& parameters, const string& prefix)
            : Command(command), Parameters(parameters), Prefix(prefix) {}
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
        const string& getCommand() const { return Command; }
        const string& getParameters() const { return Parameters; }
        const string& getPrefix() const { return Prefix; }
        static bool parse(const string &line, CommandList &commands);
        static bool hasParam(const CommandPairVector &command, ParsedParamId id);
        static const string &getParam(const CommandPairVector &command, ParsedParamId id);
};