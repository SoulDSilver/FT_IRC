#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "allincludes.hpp"
#include "Client.hpp"
#include "Channel.hpp"


class Commands
{
    private:
        std::string command;
        std::string parameters;
        std::string prefix;

    public:
        Commands();
        Commands(const std::string& command, const std::string& parameters, const std::string& prefix)
            : command(command), parameters(parameters), prefix(prefix) {}
        Commands(const Commands& other);
        Commands& operator=(const Commands& other);
        ~Commands();
        void TOPIC(const Channel& channel, const std::string& topic);
        void MODE(const Channel& channel, const std::string& mode);
        void JOIN(const Channel& channel, const Client& client);
        void INVITE(const Channel& channel, const Client& target);
        void KICK(const Channel& channel, const Client& client, const std::string& reason);
        const std::string& getCommand() const { return command; }
        const std::string& getParameters() const { return parameters; }
        const std::string& getPrefix() const { return prefix; }
};

#endif