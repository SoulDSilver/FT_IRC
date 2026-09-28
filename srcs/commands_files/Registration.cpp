#include "Server.hpp"

namespace Verify
{
	bool compareWithStrcmp(const string &provided, const string &expected)
	{
		return (strcmp(provided.c_str(), expected.c_str()) == 0);
	}
	bool constantTimeEquals(const string &provided, const string &expected)
	{
		size_t maxLength = provided.size() > expected.size()
							   ? provided.size()
							   : expected.size();
		volatile unsigned char difference = 0;

		for (size_t i = 0; i < maxLength; i++)
		{
			unsigned char providedChar = 0;
			unsigned char expectedChar = 0;
			if (i < provided.size())
				providedChar = static_cast<unsigned char>(provided[i]);
			if (i < expected.size())
				expectedChar = static_cast<unsigned char>(expected[i]);
			difference |= static_cast<unsigned char>(providedChar ^ expectedChar);
		}
		if (provided.size() != expected.size())
			difference |= 1;
		return (difference == 0);
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
		return (!username.empty() && username.find_first_of(" \t\r\n:") == string::npos);
	}

}

void Server::sendNumericReply(Client &client, const string &code,
							  const string &parameters)
{
	string nickname = client.getNick();
	if (nickname.empty())
		nickname = "*";

	string reply = ":" + name + " " + code + " " + nickname;
	if (!parameters.empty())
		reply += " " + parameters;
	reply += "\r\n";
	send(client.getFd(), reply.c_str(), reply.length(), 0);
}

void Server::handlePass(Client &client, const string &parameters)
{
	if (parameters.empty())
	{
		sendNumericReply(client, "461", "PASS :Not enough parameters");
		return;
	}
	if (!Verify::compareWithStrcmp(parameters, password))
	{
		sendNumericReply(client, "464", ":Password incorrect");
		return;
	}

	client.setPasswordAccepted();
}

void Server::handleNick(Client &client, const string &parameters)
{
	string nickname = parameters;
	bool valid = !nickname.empty() && nickname.find_first_of(" \t\r\n:") == string::npos;
	for (size_t i = 0; i < nickname.size(); i++)
	{
		if (iscntrl(static_cast<unsigned char>(nickname[i])))
			valid = false;
	}
	if (!valid)
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
}

void Server::handleUser(Client &client, const string &parameters)
{
	string username;
	string realName;
	if (!Verify::parseUserParameters(parameters, username, realName))
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
