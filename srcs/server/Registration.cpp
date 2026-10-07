#include "Server.hpp"
#include "Parser.hpp"

namespace Verify
{
	enum ReturnError
	{
		RETURN_OK = 0,
		RETURN_EMPTY,
		RETURN_TOO_LONG,
		RETURN_FORBIDDEN_CHAR,
	};

	ReturnError checkError(const string &value, size_t maxLength)
	{
		if (value.empty())
			return (RETURN_EMPTY);
		if (value.size() > maxLength)
			return (RETURN_TOO_LONG);
		if (value.find_first_of(" \t\r\n:") != string::npos)
			return (RETURN_FORBIDDEN_CHAR);
		for (size_t i = 0; i < value.size(); i++)
		{
			if (iscntrl(static_cast<unsigned char>(value[i])))
				return (RETURN_FORBIDDEN_CHAR);
		}
		return (RETURN_OK);
	}

	ReturnError checkText(const string &value, size_t maxLength)
	{
		if (value.empty())
			return (RETURN_EMPTY);
		if (value.size() > maxLength)
			return (RETURN_TOO_LONG);
		for (size_t i = 0; i < value.size(); i++)
		{
			if (iscntrl(static_cast<unsigned char>(value[i])))
				return (RETURN_FORBIDDEN_CHAR);
		}
		return (RETURN_OK);
	}
}

void Server::sendNumericReply(Client &client, const string &code,
							  const string &parameters)
{
	string nickname = client.getNick();
	if (nickname.empty())
		nickname = "*";
	// ex::irc.example.com 461 * PASS :Not enough parameters
	string reply = ":" + name + " " + code + " " + nickname;
	if (!parameters.empty())
		reply += " " + parameters;
	reply += "\r\n";
	send(client.getFd(), reply.c_str(), reply.length(), 0);
}

void Server::handlePass(Client &client, const CommandPairVector &command)
{
	if (client.isRegistered())
	{
		sendNumericReply(client, "462", ":You may not reregister");
		return;
	}
	string parameters = Parser::getParam(command, PP_PASSWORD);
	if (Parser::hasParam(command, PP_UNKNOWN) || Verify::checkError(parameters, MAX_PASS_LEN) != Verify::RETURN_OK)
	{
		sendNumericReply(client, "461", "PASS :Not enough parameters");
		return;
	}
	if (password != parameters)
	{
		sendNumericReply(client, "464", ":Password incorrect");
		client.passfail = true;
		return ;
	}

	client.setPasswordAccepted();
	if (checkClientRegistered(client) == true)
	{
		client.setRegistered();
		client.setWelcome();
		welcomeMessage(client);
		cout << GRE << "Client <" << client.getFd() << "> Registered with Password: " << parameters << WHI << endl;
	}
}

void Server::handleNick(Client &client, const CommandPairVector &command)
{
	string nickname = Parser::getParam(command, PP_NICK);
	if (Parser::hasParam(command, PP_UNKNOWN) || Verify::checkError(nickname, MAX_NICK_LEN) != Verify::RETURN_OK)
	{
		string value = nickname.empty() ? "*" : nickname;
		sendNumericReply(client, "432", value + " :Erroneous nickname");
		return;
	}

	for (map<int, Client>::const_iterator it = clients.begin();
		 it != clients.end(); ++it)
	{
		if (it->first != client.getFd() && it->second.getNick() == nickname)
		{
			sendNumericReply(client, "433",
							 nickname + " :Nickname is already in use");
			return;
		}
	}

	client.setNick(nickname);
	client.setHasNick();
	if (checkClientRegistered(client) == true)
	{
		client.setRegistered();
		client.setWelcome();
		welcomeMessage(client);
		cout << GRE << "Client <" << client.getFd() << "> Registered with Nickname: " << nickname << WHI << endl;
	}
}

void Server::handleUser(Client &client, const CommandPairVector &command)
{
	if (client.isRegistered())
	{
		sendNumericReply(client, "462", ":You may not reregister");
		return;
	}
	if (!Parser::hasParam(command, PP_USER) || !Parser::hasParam(command, PP_USER_MODE) || !Parser::hasParam(command, PP_UNUSED) || !Parser::hasParam(command, PP_REALNAME) || Parser::hasParam(command, PP_UNKNOWN))
	{
		sendNumericReply(client, "461", "USER :Not enough parameters");
		return;
	}
	string username = Parser::getParam(command, PP_USER);
	string realName = Parser::getParam(command, PP_REALNAME);
	if (Verify::checkError(username, MAX_USER_LEN) != Verify::RETURN_OK || Verify::checkText(realName, MAX_REAL_LEN) != Verify::RETURN_OK)
	{
		sendNumericReply(client, "461", "USER :Not enough parameters");
		return;
	}
	client.setUsername(username);
	client.setRealName(realName);
	client.setHasUsername();
	if (checkClientRegistered(client) == true)
	{
		client.setRegistered();
		client.setWelcome();
		welcomeMessage(client);
		cout << GRE << "Client <" << client.getFd() << "> Registered with Username: " << username << WHI << endl;
	}
}

void Server::handleQuit(Client &client, const CommandPairVector &command)
{
	string reason = Parser::getParam(command, PP_REASON);
	if (reason.empty())
		reason = Parser::getParam(command, PP_UNKNOWN);
	if (reason.empty())
		reason = "Client quit";

	string reply = "ERROR :" + reason + "\r\n";
	send(client.getFd(), reply.c_str(), reply.length(), 0);
	removeClients(client.getFd());
}
