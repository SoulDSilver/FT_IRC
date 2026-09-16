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
    
    int server_fd = listen_socket(std::atoi(av[1]));
    int client_fd = -1;
    char buffer[512];

    if (server_fd == -1)
        return 1;

    while (1)
    {
        fd_set read_fds;
        int max_fd = server_fd;

        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        if (client_fd != -1)
        {
            FD_SET(client_fd, &read_fds);
            if (client_fd > max_fd)
                max_fd = client_fd;
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            if (errno == EINTR)
                continue;
            std::cerr << "select" << std::endl;
            break;
        }

        if (FD_ISSET(server_fd, &read_fds))
        {
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd == -1)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    std::cerr << "accept" << std::endl;
            }
            else
                std::cout << "client connected (fd " << client_fd << ")" << std::endl;
        }

        if (client_fd != -1 && FD_ISSET(client_fd, &read_fds))
        {
            ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read > 0)
            {
                buffer[bytes_read] = '\0';
                std::cout << "received: " << buffer << std::endl;
                send(client_fd, buffer, bytes_read, 0);
            }
            else
            {
                if (bytes_read == -1)
                    std::cerr << "recv" << std::endl;
                else
                    std::cout << "client disconnected" << std::endl;
                close(client_fd);
                client_fd = -1;
            }
        }
    }

    if (client_fd != -1)
        close(client_fd);
    close(server_fd);
    return 0;
}
