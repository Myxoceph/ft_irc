#include "Server.hpp"

Server::Server(const std::string& port, const std::string& pwd)
{
	if (!checkPort(port))
		throw std::invalid_argument(RED"Invalid port.\n" GREEN"Usage: " WHITE"\"./ft_irc\" \"0 < port < 65536\" \"password\"" RESET);
	this->pwd = pwd;
	initServer(port);
}

Server::~Server()
{

}

std::string remove_escape_chars(const char* input) {
	std::string result;
	for (const char* p = input; *p != '\0'; ++p) {
		if (*p != '\n' && *p != '\r' && *p != '\t' &&
			*p != '\v' && *p != '\f' && *p != '\b') {
			result += *p;
		}
	}
	std::cout << "Removing escape characters: " << result << std::endl;
	return result;
}

void Server::run()
{
	while (true)
	{
		int ready = poll(fds.data(), fds.size(), -1);

		if (ready == -1)
			throw std::runtime_error(RED"Error during poll: " + std::string(strerror(errno)) + RESET);

		if (fds[0].revents & POLLIN)
		{
			int client_fd = accept(server_fd, NULL, NULL);
			if (client_fd == -1)
				throw std::runtime_error(RED"Error accepting new client: " + std::string(strerror(errno)) + RESET);

			if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
				throw std::runtime_error(RED"Error setting non-blocking mode: " + std::string(strerror(errno)) + RESET);
			clients.insert(std::make_pair(client_fd, Client(client_fd)));

			struct pollfd client_pollfd;
			client_pollfd.fd = client_fd;
			client_pollfd.events = POLLIN;
			fds.push_back(client_pollfd);
	
			std::cout << "New client connected: fd = " << client_fd << std::endl;
			ready--;
		}

		for (size_t i = 1; i < fds.size() && ready > 0; ++i)
		{
			if (fds[i].revents & (POLLHUP | POLLERR))
			{
				std::cout << "Client error/disconnect: fd = " << fds[i].fd << std::endl;
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				--i;
				ready--;
				continue;
			}

			if (fds[i].revents & POLLIN)
			{
				char buffer[1024];
				ssize_t n;

				while ((n = read(fds[i].fd, buffer, sizeof(buffer) - 1)) > 0)
				{
					buffer[n] = '\0';

					Client& client = clients.at(fds[i].fd);
					client.appendToBuffer(buffer);
					client.setPwd(pwd);

					std::string message;
					while (client.hasFullMessage(message))
					{
						handleClientMessage(client, message);
						std::cout << "IRC message from {" << fds[i].fd << "} : [" << message << "]" << std::endl;
					}
				}
				if (n == 0)
				{
					std::cout << "Client disconnected: fd = " << fds[i].fd << std::endl;
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					--i;
				}
				else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
					throw std::runtime_error(RED"Error reading from client: " + std::string(strerror(errno)) + RESET);
				ready--;
			}
		}
	}
}



// void Server::run()
// {
// 	while (true)
// 	{
// 		int ready = poll(fds.data(), fds.size(), -1);

// 		if (ready == -1)
// 			throw std::runtime_error(RED"Error during poll: " + std::string(strerror(errno)) + RESET);

// 		if (fds[0].revents & POLLIN)
// 		{
// 			int client_fd = accept(server_fd, NULL, NULL);
// 			clients.insert(std::make_pair(client_fd, Client(client_fd)));

// 			if (client_fd == -1)
// 				throw std::runtime_error(RED"Error accepting new client: " + std::string(strerror(errno)) + RESET);
// 			else
// 			{
// 				struct pollfd client_pollfd;
// 				client_pollfd.fd = client_fd;
// 				client_pollfd.events = POLLIN;
// 				fds.push_back(client_pollfd);
	
// 				std::cout << "New client connected: fd=" << client_fd << std::endl;
// 			}
// 			ready--;
// 		}
	
// 		for (size_t i = 1; i < fds.size() && ready > 0; ++i)
// 		{
// 			if (fds[i].revents & POLLIN)
// 			{
// 				char buffer[1024];
// 				ssize_t n = read(fds[i].fd, buffer, sizeof(buffer) - 1);
// 				if (n <= 0)
// 				{
// 					if (n == 0)
// 						std::cout << "Client disconnected: fd= " << fds[i].fd << std::endl;
// 					else
// 						throw std::runtime_error(RED"Error reading from client: " + std::string(strerror(errno)) + RESET);
// 					close(fds[i].fd);
// 					fds.erase(fds.begin() + i);
// 					--i;
// 				}
// 				else
// 				{
// 					buffer[n] = '\0';
// 					Client& client = clients.at(fds[i].fd);
// 					client.appendToBuffer(buffer);
// 					handleClientMessage(client);
// 					std::cout << "Received from fd " << fds[i].fd << ": " << remove_escape_chars(buffer) << std::endl;
// 				}
// 				ready--;
// 			}
// 		}
// 	}
// }


bool Server::checkPort(const std::string& port)
{
	for (size_t i = 0; i < port.size(); i++)
	{
		if (!std::isdigit(port[i]))
			return (false);
	}
	if (port.size() > 5)
		return (false);
	else if (std::strtol(port.c_str(), NULL, 10) <= 0 || std::strtol(port.c_str(), NULL, 10) > 65535)
		return (false);
	return (true);
}

void Server::send_message(int client_fd, const std::string& message)
{
	std::string msg = message + "\r\n";
	send(client_fd, msg.c_str(), msg.length(), 0);
}

// void Server::handleCommand(Client& client, const std::string& line)
// {
// 	std::istringstream iss(line);
// 	std::string cmd;
// 	iss >> cmd;

// 	std::cout << "Handling command: " << cmd << std::endl;
// 	if (cmd == "NICK")
// 	{
// 		std::string nick;
// 		iss >> nick;
// 		client.setNickname(nick);
// 		std::cout << "Nickname set to: " << nick << std::endl;
// 	}
// 	else if (cmd == "USER")
// 	{
// 		std::string user;
// 		iss >> user;
// 		client.setUsername(user);
// 		std::cout << "Username set to: " << user << std::endl;
// 	}
// 	else if (cmd == "PING")
// 	{
// 		std::string token;
// 		iss >> token;
// 		std::string pong = "PONG " + token + "\r\n";
// 		write(client.getFd(), pong.c_str(), pong.length());
// 	}
// 	else if (cmd == "JOIN")
// 	{
// 		std::string channel;
// 		iss >> channel;
// 		if (channel.empty())
// 		{
// 			std::string msg = "JOIN command requires a channel name.\r\n";
// 			write(client.getFd(), msg.c_str(), msg.length());
// 			return;
// 		}
// 		client.joinChannel(channel);
// 		std::cout << "Client joined channel: " << channel << std::endl;
// 	}
// 	else if (cmd == "PART")
// 	{
// 		std::string channel;
// 		iss >> channel;
// 		client.partChannel(channel);
// 		std::cout << "Client parted from channel: " << channel << std::endl;
// 	}
// 	else if (cmd == "QUIT")
// 	{
// 		std::cout << "Client quitting: fd = " << client.getFd() << std::endl;
// 		close(client.getFd());
// 		fds.erase(std::remove_if(fds.begin(), fds.end(), PollFdMatch(client.getFd())), fds.end());
		
// 	}
// 	else
// 	{
// 		std::string msg = "Unknown command: " + cmd + "\r\n";
// 		write(client.getFd(), msg.c_str(), msg.length());
// 	}
// }


void Server::handleClientMessage(Client& client, std::string& msg)
{
	std::string &buffer = msg;
	size_t pos;
	while ((pos = buffer.find("\r\n")) != std::string::npos)
	{
		std::string line = buffer.substr(0, pos);
		buffer.erase(pos, pos + 2);
		Commands commands(clients, channels);
		commands.executeCommand(line, client);
	}
}


void Server::initServer(const std::string& port)
{
	int opt;
	struct sockaddr_in address;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		throw std::runtime_error(RED"Error: socket creation failed " + std::string(strerror(errno)) + RESET);

	opt = 1;
	if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) && close(server_fd))
		throw std::runtime_error(RED"Error: setsockopt failed " + std::string(strerror(errno)) + RESET);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(std::strtol(port.c_str(), NULL, 10));

	if ((bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) && close(server_fd))
		throw std::runtime_error(RED"Error: bind failed " + std::string(strerror(errno)) + RESET);

	if ((listen(server_fd, 10) < 0) && close(server_fd))
		throw std::runtime_error(RED"Error: listen failed " + std::string(strerror(errno)) + RESET);

	std::cout << BLUE"Server is running on port " << port << RESET << std::endl;

	struct pollfd server_pollfd;
	server_pollfd.fd = server_fd;
	server_pollfd.events = POLLIN;
	fds.push_back(server_pollfd);
}

// void Server::initCmds()
// {
// 	usrCmds["PASS"];
// 	usrCmds["NICK"];
// 	usrCmds["USER"];
// 	usrCmds["CREATE"];
// 	usrCmds["JOIN"];
// 	usrCmds["QUIT"];

// 	chCmds["DELETE"];
// 	chCmds["LEAVE"];
// 	chCmds["ADDOP"];
// 	chCmds["KICK"];
// 	chCmds["LSTMEMBERS"];

// 	cmds["PRIVMSG"];
// 	cmds["PING"];
// 	cmds["LIST"];
// 	cmds["HELP"];
// }
