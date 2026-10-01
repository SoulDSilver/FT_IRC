#include "Commands.hpp"

namespace
{
    // Numero maximo de parametros que um comando pode declarar.
    const size_t MAX_SPEC_PARAMS = 4;

    // Descreve a "forma" de um comando IRC:
    //  - name:        nome do comando (ex.: "PRIVMSG");
    //  - paramCount:  quantos parametros o comando espera, contando o trailing;
    //  - ids:         identificador de cada parametro, na ordem em que aparecem;
    //  - hasTrailing: se true, o ULTIMO id e' o texto livre (o que vem apos ':').
    struct CommandSpec
    {
        const char     *name;
        size_t          paramCount;
        ParsedParamId   ids[MAX_SPEC_PARAMS];
        bool            hasTrailing;
    };

    // Tabela de comandos suportados. Para adicionar um comando novo basta
    // incluir uma linha aqui, sem alterar a logica de parsing.
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
        { "KICK",    3, { PP_CHANNEL, PP_TARGET, PP_REASON },               true  }
    };

    const size_t COMMAND_TABLE_SIZE =
        sizeof(COMMAND_TABLE) / sizeof(COMMAND_TABLE[0]);

    // Procura a especificacao de um comando. Retorna NULL se nao for suportado.
    const CommandSpec *findCommandSpec(const string &command)
    {
        for (size_t i = 0; i < COMMAND_TABLE_SIZE; i++)
        {
            if (command == COMMAND_TABLE[i].name)
                return (&COMMAND_TABLE[i]);
        }
        return (NULL);
    }

    // Avanca 'pos' ate o primeiro caractere que nao seja espaco, sem passar
    // de 'limit'.
    size_t skipSpaces(const string &str, size_t pos, size_t limit)
    {
        while (pos < limit && str[pos] == ' ')
            pos++;
        return (pos);
    }

    // Procura o texto livre (apos o primeiro ':'). Se existir, adiciona-o
    // a 'params' com o id informado.
    // Retorna a posicao ate onde os parametros posicionais podem ir:
    // a posicao do ':' se houver trailing, ou o fim da string caso contrario.
    size_t extractTrailing(const string &parameters, ParsedParamId trailingId,
        vector<CommandPair> &params)
    {
        size_t colon = parameters.find(':');

        if (colon == string::npos)
            return (parameters.size());
        params.push_back(make_pair(trailingId, parameters.substr(colon + 1)));
        return (colon);
    }

    // Separa por espacos os parametros posicionais em [0, limit), associando
    // cada token ao id correspondente. Retorna a posicao onde parou.
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

// Converte a parte de uma linha IRC que segue o nome do comando em uma lista
// de pares (id, valor).
//
// Formato aceito:  <param1> <param2> ... [:<texto livre>]
//   - Parametros posicionais sao separados por espacos.
//   - Se o comando aceita trailing, tudo apos o primeiro ':' e' o ultimo
//     parametro (pode conter espacos).
//   - Sobras que nao correspondem a nenhum parametro conhecido sao devolvidas
//     com o id PP_UNKNOWN, para que o chamador decida como tratar.
//
// Retorna um vetor vazio se o comando nao for suportado.
//
// Nota: o par do trailing, quando presente, e' inserido ANTES dos posicionais
// (comportamento original preservado).
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

    // 1. Texto livre (se aplicavel): delimita ate onde vao os posicionais.
    if (spec->hasTrailing)
        limit = extractTrailing(parameters, spec->ids[spec->paramCount - 1], params);

    // 2. Parametros posicionais, separados por espaco.
    size_t pos = extractPositional(parameters, limit, spec->ids,
        positionalCount, params);

    // 3. Qualquer resto nao previsto pelo comando e' marcado como desconhecido.
    pos = skipSpaces(parameters, pos, limit);
    if (pos < limit)
        params.push_back(make_pair(PP_UNKNOWN, parameters.substr(pos, limit - pos)));

    return (params);
}

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

    commands.push_back(make_pair(command, buildParams(command, parameters)));
    return (true);
}

bool Commands::hasParam(const CommandPairVector &command, ParsedParamId id)
{
    for (vector<CommandPair>::const_iterator it = command.second.begin();
        it != command.second.end(); ++it)
    {
        if (it->first == id)
            return (true);
    }
    return (false);
}

const string &Commands::getParam(const CommandPairVector &command, ParsedParamId id)
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
