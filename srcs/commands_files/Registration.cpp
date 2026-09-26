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
		? provided.size() : expected.size();
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

}

void Server::sendNumericReply(Client &client, const string &code,
	const string &parameters)
{
	string nickname = client.getNick();
	if (nickname.empty())
		nickname = "*";

	string reply = ":" + Name + " " + code + " " + nickname;
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
	if (!Verify::compareWithStrcmp(parameters, Password))
	{
		sendNumericReply(client, "464", ":Password incorrect");
		return;
	}

	client.setPasswordAccepted();
}

void Server::handleNick(Client &client, const string &parameters)
{
	string nickname = parameters;
	bool valid = !nickname.empty()
		&& nickname.find_first_of(" \t\r\n:") == string::npos;
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

void Server::handleQuit(Client &client, const string &parameters)
{
	string reason = parameters;
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	if (reason.empty())
		reason = "Client quit";

	string reply = "ERROR :" + reason + "\r\n";
	send(client.getFd(), reply.c_str(), reply.length(), 0);

	for (map<string, Channel>::iterator it = Channels.begin();
		it != Channels.end(); ++it)
		it->second.removeClient(client);

	map<string, Channel>::iterator channel = Channels.begin();
	while (channel != Channels.end())
	{
		if (channel->second.getClients().empty())
		{
			map<string, Channel>::iterator empty = channel++;
			Channels.erase(empty);
		}
		else
			++channel;
	}
}
