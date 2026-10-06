#ifndef IRC_HPP
#define IRC_HPP

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include <cctype>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <algorithm>
#include <unistd.h>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RED "\e[1;31m" //-> for red color
#define WHI "\e[0;37m" //-> for white color
#define GRE "\e[1;32m" //-> for green color
#define YEL "\e[1;33m" //-> for yellow color

using namespace std;

//usar o proprio enum ao invez de int
//typedef pair<ParsedParamId, string> Parameters;
typedef pair<int, string> Parameters;
typedef pair<string, vector<Parameters> > CommandPairVector;
typedef vector<CommandPairVector> CommandList;

// Enum para identificar cada parâmetro parseado
enum ParsedParamId
{
	PP_UNKNOWN = 0,
	PP_USER = 1,	// nome de usuário no comando USER
	PP_NICK = 2,	// apelido no comando NICK
	PP_MODE = 3,	// modos no comando MODE
	PP_CHANNEL = 4, // nome do canal
	PP_TARGET = 5,	// alvo (usuário) para PRIVMSG/INVITE/KICK
	PP_MESSAGE = 6, // corpo da mensagem (PRIVMSG/QUIT)
	PP_REASON = 7,	// motivo (KICK/PART/QUIT)
	PP_TOPIC = 8	// texto do tópico (TOPIC)
};

#endif