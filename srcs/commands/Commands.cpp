#include "Commands.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void Commands::PING(Server &server, const string &serverName, int fd)
{
	if (serverName.empty())
		return;
	server.pongmessage(fd, serverName);
}

void Commands::PRIVMSG(Server &server, const string &target, const string &message, int fd)
{
	cout << GRE << "target" << target << WHI << endl;
	if (!server.channelExists(target))
		return;

	if (!server.isClientInChannel(target, fd))
		return;

	server.broadcastToChannel(target, message, fd);
}

void Commands::JOIN(Server &server, const string &channelName, int fd)
{
	string menssage = ":<nick>!<user>@<host> JOIN :#" + channelName;
	if (server.channelExists(channelName) == 0)
	{
		server.createChannel(channelName, fd);
		server.broadcastToChannel(channelName, menssage, fd);
	}
	else if (server.channelExists(channelName) == 1)
	{
		server.addClientToChannel(channelName, fd);
		server.broadcastToChannel(channelName, menssage, fd);
		cout << GRE << "Client <" << fd << "> Joined Channel <" << channelName << ">" << WHI << endl;
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
	{
		server.sendPartMessages(channelName, fd, reason);
	}
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