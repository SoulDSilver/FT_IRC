#include "Commands.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void Commands::PING(Server &server, const string &serverName, int fd)
{
	if (serverName.empty())
		return;
	server.pongmessage(fd, serverName);
}

void Commands::PRIVMSG(Server &server, const string &target, const string &message,  Client &client)
{
	if (target[0] == '#')
	{
		if (server.channelExists(target) == 0)
		{
			server.sendNumericReply(client, "403", target + " :No such channel");
			return;
		}
		if (!server.isClientInChannel(target, client.getFd()))
		{
			server.sendNumericReply(client, "404", target + " :Cannot send to channel");
			return;
		}

		const Channel *channel = server.getChannel(target);
	
		channel->broadcastMessage(message, client);
		return;
	}

	
	const Client *clientPtr = server.getClientByNick(target);
	

	if (clientPtr == NULL)
	{
		cout << RED << "Client <" << client.getFd() << "> tried to send message to non-existent nick <" << target << ">" << WHI << endl;
		server.sendNumericReply(client, "401", target + " :No such nick/channel");
		return;
	}

	string line = ":" + client.getNick() + "!" + client.getUsername() + "@" + client.getIpAddr() + " PRIVMSG " + target + " :" + message + "\r\n";
	send(clientPtr->getFd(), line.c_str(), line.length(), 0);
	cout << GRE << "Client <" << client.getFd() << "> Sent Message to <" << target << ">: " << message << WHI << endl;
	return;
}

void Commands::JOIN(Server &server, const string &channelName,  Client &client)
{
	if (server.channelExists(channelName) == 0)
	{
		server.createChannel(channelName, client);
		server.sendChannelJoinMessages(channelName, client.getFd());
	}
	else if (server.channelExists(channelName) == 1)
	{
		server.addClientToChannel(channelName, client.getFd());
		server.sendChannelJoinMessages(channelName, client.getFd());
		cout << GRE << "Client <" << client.getFd() << "> Joined Channel <" << channelName << ">" << WHI << endl;
	}
}

void Commands::PART(Server &server, string &channelName, int fd)
{
	size_t reasonPosition = channelName.find(" :");
	string reason;
	if (reasonPosition != string::npos)
	{
		reason = channelName.substr(reasonPosition + 2);
		channelName = channelName.substr(0, reasonPosition);
	}
	if (server.channelExists(channelName) == 1 && server.isClientInChannel(channelName, fd))
		server.sendPartMessages(channelName, fd, reason);
	
}

void Commands::TOPIC(Server &server, const string &target, int fd)
{
	if (!server.channelExists(target))
		return;
	server.sendTopicMessages(target, fd);
}
/*
void Commands::processCommands(Server &server, const CommandList &parsedCommands, int fd)
{
	for (size_t i = 0; i < parsedCommands.size(); i++)
	{
		if (parsedCommands[i].first == "PASS")
		{
			pass(server, "parames", fd);
		}
		else if (parsedCommands[i].first == "NICK")
		{
			nick(server, "parames", fd);
		}
		else if (parsedCommands[i].first == "USER")
		{
			user(server, "parames", fd);
		}
		else if (parsedCommands[i].first == "JOIN")
		{
			join(server, "parames", fd);
		}
			else if (parsedCommands[i].first == "PART")
			{
				part(server, "parames", fd);
			}
			else if (parsedCommands[i].first == "PRIVMSG")
			{
				privmsg(server, "parames", "other argument", fd);
			}
			else if (parsedCommands[i].first == "KICK")
			{
				kick(channel, client, "other argument");
			}
			else if (parsedCommands[i].first == "INVITE")
			{
				invite(channel, target);
			}
			else if (parsedCommands[i].first == "TOPIC")
			{
				topic(server, "parames", fd);
			}
			else if (parsedCommands[i].first == "MODE")
			{
				mode(channel, "parames");
			}
else if (parsedCommands[i].first == "QUIT")
{
	quit(server, "parames", fd);
}
else if (parsedCommands[i].first == "PING")
{
	ping(server, "parames", fd);
}
}
}
*/