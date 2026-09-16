#include "irc.hpp"

class Client;

class Channel
{
    private:
        std::string name;
        std::vector<Client> clients;

    public:
        Channel();
        Channel(const std::string& name);
        Channel(const Channel& other);
        Channel& operator=(const Channel& other);
        ~Channel();
        const std::string& getName() const { return name; }
        const std::vector<Client>& getClients() const { return clients; }
        void addClient(const Client& client) { clients.push_back(client); }
        void removeClient(const Client& client);
};