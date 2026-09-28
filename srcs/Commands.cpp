#include "Commands.hpp"

Commands::Commands() : Command(""), Parameters(""), Prefix("") {}

Commands::Commands(const Commands &other)
    : Command(other.Command), Parameters(other.Parameters), Prefix(other.Prefix) {}

Commands &Commands::operator=(const Commands &other)
{
    if (this != &other)
    {
        Command = other.Command;
        Parameters = other.Parameters;
        Prefix = other.Prefix;
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
