#include "Server.hpp"

int main(int ac, char **av)
{
    if (ac != 3 || !av[1] || !av[2])
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    if(std::atoi(av[1]) <= 0 || std::atoi(av[1]) > 65535)
    {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }
    
    Server server(std::atoi(av[1]));
    int server_fd = server.getListenFd();
   // int client_fd = -1;
   // char buffer[512];

    if (server_fd == -1)
        return 1;

    while(1){
     
        break;
    }

    return 0;
}
