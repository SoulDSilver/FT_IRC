#include "Server.hpp"

namespace Verify
{
	enum ReturnError
	{
		RETURN_OK = 0,
		RETURN_EMPTY,
		RETURN_TOO_LONG,
		RETURN_FORBIDDEN_CHAR
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

	bool parseUserParameters(const string &parameters, string &username,
							 string &realName)
	{
		size_t first = parameters.find(' ');
		if (first == string::npos || first == 0)
			return (false);

		size_t second = parameters.find(' ', first + 1);
		if (second == string::npos || second == first + 1)
			return (false);

		size_t third = parameters.find(' ', second + 1);
		if (third == string::npos || third == second + 1)
			return (false);

		size_t colon = parameters.find(':', third + 1);
		if (colon == string::npos || colon + 1 >= parameters.size())
			return (false);

		username = parameters.substr(0, first);
		realName = parameters.substr(colon + 1);
		return (true);
	}

}

void Server::sendNumericReply(Client &client, const string &code,
							  const string &parameters)
{
	string nickname = client.getNick();
	if (nickname.empty())
		nickname = "*";
	// ex::irc.example.com 461 * PASS :Not enough parameters
	string reply = ":" + Name + " " + code + " " + nickname;
	if (!parameters.empty())
		reply += " " + parameters;
	reply += "\r\n";
	send(client.getFd(), reply.c_str(), reply.length(), 0);
}

void Server::handlePass(Client &client, const string &parameters)
{
	if (Verify::checkError(parameters, MAX_PASS_LEN) != Verify::RETURN_OK)
	{
		sendNumericReply(client, "461", "PASS :Not enough parameters");
		return;
	}
	if (Password != parameters)
	{
		sendNumericReply(client, "464", ":Password incorrect");
		return;
	}

	client.setPasswordAccepted();
}

void Server::handleNick(Client &client, const string &parameters)
{
	string nickname = parameters;
	if (Verify::checkError(nickname, MAX_NICK_LEN) != Verify::RETURN_OK)
	{
		string value = nickname.empty() ? "*" : nickname;
		sendNumericReply(client, "432", value + " :Erroneous nickname");
		return;
	}

	for (map<int, Client>::const_iterator it = Clients.begin();
		 it != Clients.end(); ++it)
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
}

void Server::handleUser(Client &client, const string &parameters)
{
	string username;
	string realName;
	if (!Verify::parseUserParameters(parameters, username, realName) || Verify::checkError(username, MAX_USER_LEN) != Verify::RETURN_OK || Verify::checkText(realName, MAX_REAL_LEN) != Verify::RETURN_OK)
	{
		sendNumericReply(client, "461", "USER :Not enough parameters");
		return;
	}

	client.setUsername(username);
	client.setRealName(realName);
	client.setHasUsername();
}

void Server::handleQuit(Client &client, const string &parameters)
{
	string reason = parameters;
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	if (reason.empty())
		reason = "Client quit";

	string reply = "ERROR :" + reason + "\r\n";
	send(client.getFd(), reply.c_str(), reply.length(), 0);
	removeClients(client.getFd());
}
