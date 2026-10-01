#include "Parser.hpp"

namespace
{
    const size_t MAX_SPEC_PARAMS = 4;

    struct CommandSpec
    {
        const char     *name;
        size_t          paramCount;
        ParsedParamId   ids[MAX_SPEC_PARAMS];
        bool            hasTrailing;
    };

    const CommandSpec COMMAND_TABLE[] =
    {
        { "PASS",    1, { PP_PASSWORD },                                    false },
        { "NICK",    1, { PP_NICK },                                        false },
        { "USER",    4, { PP_USER, PP_USER_MODE, PP_UNUSED, PP_REALNAME },  true  },
        { "QUIT",    1, { PP_REASON },                                      true  },
        { "JOIN",    1, { PP_CHANNEL },                                     false },
        { "PART",    2, { PP_CHANNEL, PP_REASON },                          true  },
        { "PRIVMSG", 2, { PP_TARGET, PP_MESSAGE },                          true  },
        { "MODE",    2, { PP_CHANNEL, PP_MODE },                            false },
        { "TOPIC",   2, { PP_CHANNEL, PP_TOPIC },                           true  },
        { "INVITE",  2, { PP_TARGET, PP_CHANNEL },                          false },
{ "KICK",    3, { PP_CHANNEL, PP_TARGET, PP_REASON },  true  },
	{ "CAP",     1, { PP_TARGET },                           false },
	{ "PING",    1, { PP_TARGET },                           false }
};

    const size_t COMMAND_TABLE_SIZE =
        sizeof(COMMAND_TABLE) / sizeof(COMMAND_TABLE[0]);

    const CommandSpec *findCommandSpec(const string &command)
    {
        for (size_t i = 0; i < COMMAND_TABLE_SIZE; i++)
        {
            if (command == COMMAND_TABLE[i].name)
                return (&COMMAND_TABLE[i]);
        }
        return (NULL);
    }

    size_t skipSpaces(const string &str, size_t pos, size_t limit)
    {
        while (pos < limit && str[pos] == ' ')
            pos++;
        return (pos);
    }

    size_t extractPositional(const string &parameters, size_t limit,
        const ParsedParamId *ids, size_t positionalCount,
        vector<CommandPair> &params)
    {
        size_t pos = 0;

        for (size_t i = 0; i < positionalCount && pos < limit; i++)
        {
            pos = skipSpaces(parameters, pos, limit);
            if (pos >= limit)
                break;

            size_t space = parameters.find(' ', pos);
            if (space == string::npos || space > limit)
                space = limit;

            params.push_back(make_pair(ids[i], parameters.substr(pos, space - pos)));
            pos = space + 1;
        }
        return (pos);
    }
}

static vector<CommandPair> buildParams(const string &command,
    const string &parameters)
{
    const CommandSpec *spec = findCommandSpec(command);
    if (spec == NULL)
        return (vector<CommandPair>());

    vector<CommandPair> params;
    size_t positionalCount = spec->hasTrailing ? spec->paramCount - 1
                                               : spec->paramCount;
    size_t limit = parameters.size();
    size_t colon;
    if (spec->hasTrailing)
    {
        colon = parameters.find(':');
        if (colon != string::npos)
            limit = colon;
    }

    size_t pos = extractPositional(parameters, limit, spec->ids,
        positionalCount, params);
    if (spec->hasTrailing && colon != string::npos)
        params.push_back(make_pair(spec->ids[positionalCount], parameters.substr(colon + 1)));

    pos = skipSpaces(parameters, pos, limit);
    if (pos < limit)
        params.push_back(make_pair(PP_UNKNOWN, parameters.substr(pos, limit - pos)));

    return (params);
}

bool Parser::parse(const string &line, CommandList &commands)
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

    commands.push_back(make_pair(command, buildParams(command, parameters)));
    return (true);
}

bool Parser::hasParam(const CommandPairVector &command, ParsedParamId id)
{
    for (vector<CommandPair>::const_iterator it = command.second.begin();
        it != command.second.end(); ++it)
    {
        if (it->first == id)
            return (true);
    }
    return (false);
}

const string &Parser::getParam(const CommandPairVector &command, ParsedParamId id)
{
    static const string Empty;
    for (vector<CommandPair>::const_iterator it = command.second.begin();
        it != command.second.end(); ++it)
    {
        if (it->first == id)
            return (it->second);
    }
    return (Empty);
}
