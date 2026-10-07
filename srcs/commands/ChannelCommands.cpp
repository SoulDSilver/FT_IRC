#include "Server.hpp"
#include "Parser.hpp"

void Server::handleJoin(Client &client, const CommandPairVector &command)
{
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
	Commands::JOIN(*this, channelName, client);
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
		sendNumericReply(client, "442", channelName + " :You're not on that channel");
		return;
	}
	Commands::PART(*this, channelName, client.getFd());
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

	Commands::PRIVMSG(*this, target, message, client);
}
