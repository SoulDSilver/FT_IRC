#include "Server.hpp"
#include "Parser.hpp"

void Server::handleJoin(Client &client, const CommandPairVector &command)
{
	cout << "meu nome real: " << client.getRealName() << endl;
	if (!client.isRegistered())
	{
		sendNumericReply(client, "451", ":You have not registered");
		return;
	}

	string channelName = Parser::getParam(command, PP_CHANNEL);
	if (channelName.empty() || channelName[0] != '#')
	{
		sendNumericReply(client, "403", channelName + " :No such channel");
		return;
	}

	if (channels.count(channelName) == 0)
	{
		createChannel(channelName, client);
		channels.at(channelName).sendJoinMessages(client);
		cout << GRE << "Client <" << client.getFd() << "> Joined Channel <"
			 << channelName << ">" << WHI << endl;
		return;
	}

	if (!isClientInChannel(channelName, client.getFd()))
		channels.at(channelName).addClient(client);

	channels.at(channelName).sendJoinMessages(client);
	cout << GRE << "Client <" << client.getFd() << "> Joined Channel <"
		 << channelName << ">" << WHI << endl;
}

void Server::handlePart(Client &client, const CommandPairVector &command)
{
	if (!client.isRegistered())
	{
		sendNumericReply(client, "451", ":You have not registered");
		return;
	}

	string channelName = Parser::getParam(command, PP_CHANNEL);
	string reason = Parser::getParam(command, PP_REASON);

	if (channelName.empty() || channels.count(channelName) == 0)
	{
		sendNumericReply(client, "403", channelName + " :No such channel");
		return;
	}
	if (!isClientInChannel(channelName, client.getFd()))
	{
		sendNumericReply(client, "442", channelName
			+ " :You're not on that channel");
		return;
	}

	channels.at(channelName).sendPartMessage(client, reason);
	channels.at(channelName).removeClient(client);
	cout << GRE << "Client <" << client.getFd() << "> removed from Channel <"
		 << channelName << ">" << WHI << endl;

	if (channels.at(channelName).have_any_client())
		removeChannel(channelName);
}

void Server::handlePrivmsg(Client &client, const CommandPairVector &command)
{
	if (!client.isRegistered())
	{
		sendNumericReply(client, "451", ":You have not registered");
		return;
	}

	string target = Parser::getParam(command, PP_TARGET);
	string message = Parser::getParam(command, PP_MESSAGE);

	if (target.empty() || message.empty())
	{
		sendNumericReply(client, "461", "PRIVMSG :Not enough parameters");
		return;
	}

	if (target[0] == '#')
	{
		if (channels.count(target) == 0)
		{
			sendNumericReply(client, "403", target + " :No such channel");
			return;
		}
		if (!isClientInChannel(target, client.getFd()))
		{
			sendNumericReply(client, "404", target
				+ " :Cannot send to channel");
			return;
		}
		channels.at(target).broadcastMessage(message, client.getFd());
		return;
	}

	for (map<int, Client>::const_iterator it = clients.begin();
		 it != clients.end(); ++it)
	{
		if (it->first != client.getFd() && it->second.getNick() == target)
		{
			string line = ":" + client.getNick() + "!" + client.getUsername()
				+ "@" + client.getIpAddr() + " PRIVMSG " + target
				+ " :" + message + "\r\n";
			send(it->first, line.c_str(), line.length(), 0);
			return;
		}
	}

	sendNumericReply(client, "401", target + " :No such nick/channel");
}
