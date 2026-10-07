#include "Server.hpp"
#include "Parser.hpp"

void Server::handleMode(Client &client, const CommandPairVector &command)
{
	if (!client.isRegistered())
	{
		sendNumericReply(client, "451", ":You have not registered");
		return;
	}

	string target = Parser::getParam(command, PP_CHANNEL);

	if (target.empty())
	{
		sendNumericReply(client, "461", "MODE :Not enough parameters");
		return;
	}

	if (target == client.getNick())
	{
		sendNumericReply(client, "221", "+i");
		return;
	}

	if (channels.count(target) == 1)
	{
		sendNumericReply(client, "324", target + " +nt");
		return;
	}

	sendNumericReply(client, "401", target + " :No such nick/channel");
}

void Server::handleWhois(Client &client, const CommandPairVector &command)
{
	if (!client.isRegistered())
	{
		sendNumericReply(client, "451", ":You have not registered");
		return;
	}

	string target = Parser::getParam(command, PP_TARGET);
	if (target.empty())
	{
		sendNumericReply(client, "431", ":No nickname given");
		return;
	}

	if (target == client.getNick())
	{
		string nick = client.getNick();
		string user = client.getUsername();
		string realName = client.getRealName();
		string host = client.getIpAddr();

		sendNumericReply(client, "311", nick + " " + user + " " + host + " *" + realName);
		sendNumericReply(client, "312", nick + " " + name);
		for (map<string, Channel>::const_iterator ch = channels.begin();
			 ch != channels.end(); ++ch)
		{
			if (ch->second.isClientPresent(client.getFd()))
			{
				sendNumericReply(client, "319", nick + " :" + ch->second.getName());
			}
		}
		sendNumericReply(client, "318", nick + " :End of /WHOIS list");
		return;
	}

	for (map<int, Client>::const_iterator it = clients.begin();
		 it != clients.end(); ++it)
	{
		if (it->first == client.getFd() || it->second.getNick() != target)
			continue;

		string nick = it->second.getNick();
		string user = it->second.getUsername();
		string realName = it->second.getRealName();
		string host = it->second.getIpAddr();

		sendNumericReply(client, "311", nick + " " + user + " " + host + " *" + realName);
		sendNumericReply(client, "312", nick + " " + name);
		cout << "WHOIS: " << nick << " " << user << " " << host << " *" << realName << endl;
		for (map<string, Channel>::const_iterator ch = channels.begin();
			 ch != channels.end(); ++ch)
		{
			if (ch->second.isClientPresent(it->first))
			{
				sendNumericReply(client, "319", nick + " :" + ch->second.getName());
			}
		}
		sendNumericReply(client, "318", nick + " :End of /WHOIS list");
		return;
	}

	sendNumericReply(client, "401", target + " :No such nick/channel");
}

void Server::handleMotd(Client &client, const CommandPairVector &command)
{
	(void)command;

	if (!client.isRegistered())
	{
		sendNumericReply(client, "451", ":You have not registered");
		return;
	}

	sendNumericReply(client, "375", "- " + name + " Message of the day -");
	sendNumericReply(client, "372", "- This server runs ircd-1.0");
	sendNumericReply(client, "376", ":End of /MOTD command");
}
