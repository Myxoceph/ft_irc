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
	for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
		if (it->first != -1)
			close(it->first);
	close(server_fd);
	clients.clear();
	fds.clear();
	channels.clear();
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
				if (fds[i].fd != -1)
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
					std::map<int, Client>::iterator it = clients.find(fds[i].fd);
					if (it == clients.end())
					{
						std::cerr << "Error: Client not found for fd " << fds[i].fd << std::endl;
						break;
					}
					
					Client& client = it->second;
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
					std::map<int, Client>::iterator it = clients.find(fds[i].fd);
					if (it != clients.end())
					{
						removeUser(it->second.getUsername());
						removeNick(it->second.getNickname());
						std::string quit = "QUIT\r\n";
						handleClientMessage(it->second, quit);
						clients.erase(it);
					}
					if (fds[i].fd != -1)
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

bool Server::checkPort(const std::string& port)
{
	for (size_t i = 0; i < port.size(); i++)
		if (!std::isdigit(port[i]))
			return (false);

	if (port.size() > 5)
		return (false);
	else if (std::strtol(port.c_str(), NULL, 10) <= 0 || std::strtol(port.c_str(), NULL, 10) > 65535)
		return (false);

	return (true);
}

void Server::handleClientMessage(Client& client, std::string& msg)
{
	std::string &buffer = msg;

	size_t pos;
	while ((pos = buffer.find("\r\n")) != std::string::npos)
	{
		std::string line = buffer.substr(0, pos);
		buffer.erase(pos, pos + 2);
		Commands commands(clients, channels, *this);
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
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(server_fd);
		throw std::runtime_error(RED"Error: setsockopt failed " + std::string(strerror(errno)) + RESET);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(std::strtol(port.c_str(), NULL, 10));

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		close(server_fd);	
		throw std::runtime_error(RED"Error: bind failed " + std::string(strerror(errno)) + RESET);
	}

	if (listen(server_fd, 10) < 0)
	{
		close(server_fd);
		throw std::runtime_error(RED"Error: listen failed " + std::string(strerror(errno)) + RESET);
	}

	std::cout << BLUE"Server is running on port " << port << RESET << std::endl;

	struct pollfd server_pollfd;
	server_pollfd.fd = server_fd;
	server_pollfd.events = POLLIN;
	fds.push_back(server_pollfd);
}

bool Server::addUser(std::string& user)
{
	if (std::find(userList.begin(), userList.end(), user) != userList.end())
		return false;

	userList.push_back(user);
	return true;
}

bool Server::addNick(std::string& nick)
{
	if (std::find(nickList.begin(), nickList.end(), nick) != nickList.end())
		return false;

	nickList.push_back(nick);
	return true;
}

void Server::removeUser(std::string user)
{
	std::vector<std::string>::iterator it = std::find(userList.begin(), userList.end(), user);
	if (it != userList.end())
		userList.erase(it);
}

void Server::removeNick(std::string nick)
{
	std::vector<std::string>::iterator it = std::find(nickList.begin(), nickList.end(), nick);
	if (it != nickList.end())
		nickList.erase(it);
}
