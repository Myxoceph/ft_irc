#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <vector>
#include <poll.h>
#include <map>
#include "Client.hpp"
#include "Channel.hpp"
#include <cstdlib>
#include <cctype>
#include <sstream>

#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define CYAN "\033[1;36m"
#define BLUE "\033[1;34m"
#define WHITE "\033[1;37m"
#define RED "\033[1;31m"
#define RESET "\033[0m"

class Client;

class Server
{
	private:
			int							server_fd;
			std::string					pwd;
			std::vector<struct pollfd>	fds;
			std::map<int, Client>		clients;
			bool checkPort(const std::string& port);
			void initServer(const std::string& port);
			void handleClientMessage(Client& client);
			void handleCommand(Client& client, const std::string& line);
			// void initCmds();

			// typedef void (Server::*UsrCmds)(Client*, const std::vector<std::string>&);
			// typedef void (Server::*ChCmds)(Channel*, Client*, const std::vector<std::string>&);
			// typedef void (Server::*Cmds)(Client*, const std::vector<std::string>&);

			// std::map<std::string, UsrCmds> usrCmds;
			// std::map<std::string, ChCmds> chCmds;
			// std::map<std::string, Cmds> cmds;

	public:
			Server(const std::string& port, const std::string& pwd);
			~Server();
			void run();
			void send_message(int client_fd, const std::string& message);
};

#endif
