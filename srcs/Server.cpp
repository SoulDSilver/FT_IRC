#include "Server.hpp"

template <typename T> static T max(T a, T b)
{
	return ((a > b) ? a : b);
}

int	Server::listen_socket(int listen_port)
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

Server::Server() : listen_port(0), password(""), listen_fd(-1) {}


Server::Server(int port) : listen_port(port), password("my")
{
	listen_fd = listen_socket(port);
}

Server::Server(const Server &other)
    : listen_port(other.listen_port), password(other.password), listen_fd(other.listen_fd) {}

Server& Server::operator=(const Server& other)
{
    if (this != &other)
    {
        listen_port = other.listen_port;
        //password = other.password;
        listen_fd = other.listen_fd;
    }
    return *this;
}

Server::~Server()
{
    if (listen_fd != -1)
        close(listen_fd);
}

void Server::run()
{
    (void)listen_fd;
}

int Server::getListenPort() const
{
    return listen_port;
}

int Server::getListenFd() const
{
    return listen_fd;
}
