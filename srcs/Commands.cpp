#include "Commands.hpp"

Commands::Commands() : command(""), parameters(""), prefix("") {}

Commands::Commands(const string &command, const string &parameters, const string &prefix) : command(command), parameters(parameters), prefix(prefix)
{
}

Commands::Commands(const Commands &other)
    : command(other.command), parameters(other.parameters), prefix(other.prefix) {}

Commands &Commands::operator=(const Commands &other)
{
    if (this != &other)
    {
        command = other.command;
        parameters = other.parameters;
        prefix = other.prefix;
    }
    return *this;
}

Commands::~Commands() {}

bool Commands::parse(const string &line, CommandList &commands)
{
    size_t start = line.find_first_not_of(" \t");
    if (start == string::npos)
        return (false);

    size_t end = line.find_first_of(" \t", start);
    string command = (end == string::npos) ? line.substr(start)
                                           : line.substr(start, end - start);
    for (size_t i = 0; i < command.size(); i++)
        command[i] = static_cast<char>(toupper(static_cast<unsigned char>(command[i])));

    string parameters;
    if (end != string::npos)
    {
        size_t parameterStart = line.find_first_not_of(" \t", end);
        if (parameterStart != string::npos)
            parameters = line.substr(parameterStart);
    }
    commands.push_back(make_pair(command, parameters));
    return (true);
}
