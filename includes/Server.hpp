#include "irc.hpp"


class Server
{
    private:
        int listen_port;
        std::string password;
        int listen_fd;

    public:
        Server();
        Server(int port, const std::string& password);
        Server(const Server& other);
        Server& operator=(const Server& other);
        ~Server();
        void run();
};

int listen_socket(int listen_port);