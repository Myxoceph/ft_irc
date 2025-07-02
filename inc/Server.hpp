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
	public:
		// Constructor and Destructor
		Server(const std::string& port, const std::string& pwd);
		~Server();

		// Public Methods
		void run();

	private:
		// Server Attributes
		int							server_fd;
		std::string					pwd;

		// Client and Channel Management
		std::vector<struct pollfd>	fds;
		std::map<int, Client>		clients;
		std::map<std::string, Channel> channels;
		Commands*					_commands;

		// Functor for finding pollfd by fd
		struct PollfdFinder
		{
			int fd_to_find;
			PollfdFinder(int fd) : fd_to_find(fd) {}
			bool operator()(const struct pollfd& pfd) const
			{
				return pfd.fd == fd_to_find;
			}
		};

		// Private Methods
		void initServer(const std::string& port);
		void handleNewConnection();
		void handleClientData(size_t i);
		void removeClient(size_t i, bool graceful = false);
		bool handleClientMessage(Client& client, std::string& line);
		bool checkPort(const std::string& port);
};

#endif
