#include "Commands.hpp"

Commands::Commands() : command(""), parameters(""), prefix("") {}

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
