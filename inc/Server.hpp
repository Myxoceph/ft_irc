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
#include "Parser.hpp"
#include "Commands.hpp"
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

class Channel;

class Commands;

class Server
{
	private:
			int							server_fd;
			std::string					pwd;
			std::vector<struct pollfd>	fds;
			std::map<int, Client>		clients;
			std::map<std::string, Channel> channels;
			std::vector<std::string>	userList;
			std::vector<std::string>	nickList;
			bool checkPort(const std::string& port);
			void initServer(const std::string& port);
			void handleClientMessage(Client& client, std::string& line);

	public:
			Server(const std::string& port, const std::string& pwd);
			~Server();
			void run();
			bool addUser(std::string& user);
			bool addNick(std::string& nick);
			
			void removeNick(std::string nick);
			void removeUser(std::string user);

			struct PollFdMatch
			{
				int fdToRemove;
				PollFdMatch(int fd) : fdToRemove(fd) {}
			
				bool operator()(const struct pollfd& pfd) const
				{
					return pfd.fd == fdToRemove;
				}
			};
};

#endif
