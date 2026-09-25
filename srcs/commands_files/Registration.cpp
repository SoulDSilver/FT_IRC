#include "Server.hpp"

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
