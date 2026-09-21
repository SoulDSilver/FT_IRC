#include "Server.hpp"

int	main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	if (std::atoi(av[1]) <= 0 || std::atoi(av[1]) > 65535)
	{
		std::cerr << "Invalid port number" << std::endl;
		return (1);
	}
	Server server(std::atoi(av[1]), av[2]);
	try
	{
        signal(SIGINT, Server::signalHandler); // (ctrl + c)
		signal(SIGQUIT, Server::signalHandler); // (ctrl + \)
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (-1);
	}
	return (0);
}
