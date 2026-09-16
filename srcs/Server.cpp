#include "Server.hpp"

template <typename T> static T max(T a, T b)
{
	return ((a > b) ? a : b);
}

int	listen_socket(int listen_port)
{
	struct sockaddr_in	addr;
	int					lfd;
	int					yes;

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		std::cerr << "socket" << std::endl;
		return (-1);
	}
	yes = 1;
	if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
	{
		std::cerr << "setsockopt" << std::endl;
		close(lfd);
		return (-1);
	}
	// memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(listen_port);
	addr.sin_family = AF_INET;
	if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		std::cerr << "bind" << std::endl;
		close(lfd);
		return (-1);
	}
	if (fcntl(lfd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl" << std::endl;
		close(lfd);
		return (-1);
	}
	std::cout << "accepting connections on port " << listen_port << std::endl;
	if (listen(lfd, 10) == -1)
	{
		std::cerr << "listen" << std::endl;
		close(lfd);
		return (-1);
	}
	return (lfd);
}

Server::Server()
{
}
