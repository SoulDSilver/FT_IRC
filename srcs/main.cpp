#include "irc.hpp"
#include "Server.hpp"

int main(int ac, char **av)
{
	if (ac != 3)
	{
		cerr << "Usage: ./ircserv <port> <password>" << endl;
		return (1);
	}
	if (atoi(av[1]) <= 0 || atoi(av[1]) > 65535)
	{
		cerr << "Invalid port number" << endl;
		return (1);
	}
	Server server(atoi(av[1]), av[2]);
	try
	{
		signal(SIGINT, Server::signalHandler);	// (ctrl + c)
		signal(SIGQUIT, Server::signalHandler); // (ctrl + \)
		server.run();
	}
	catch (const exception &e)
	{
		cerr << "Error: " << e.what() << endl;
		return (-1);
	}
	return (0);
}
