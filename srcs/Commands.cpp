#include "Commands.hpp"

void Commands::PING(int fd, const string& serverName){}

void Commands::PRIVMSG(const map<string, Channel>& channels, const string& target, const string& message, int fd, bool isClientInChannel){
	cout << GRE << "target" << target << WHI << endl;
	if (channels.count(target) == 1)
	{
		if (isClientInChannel)
			channels.at(target).broadcastMessage(message, fd);
	}
}
        