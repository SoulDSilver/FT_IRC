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
