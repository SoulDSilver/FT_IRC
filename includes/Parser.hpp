#pragma once

#include "irc.hpp"

class Parser
{
    public:
        static bool parse(const string &line, CommandList &commands);
        static bool hasParam(const CommandPairVector &command, ParsedParamId id);
        static const string &getParam(const CommandPairVector &command, ParsedParamId id);
};