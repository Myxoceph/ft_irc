#include "Server.hpp"

Server::Server(const std::string& port, const std::string& pwd)
{
	if (!checkPort(port))
	{
		std::cerr << RED"Invalid port.\n" << GREEN"Usage: " << WHITE"\"./ft_irc\" \"0 < port < 65536\" \"password\""RESET << std::endl;
		std::exit(1);
	}
	this->pwd = pwd;
	initServer(port);
}

Server::~Server()
{

}

void Server::run()
{
	while (true)
		{
			int ret = poll(&fds[0], fds.size(), -1);
			if (ret < 0)
			{
				std::cerr << "Poll failed: " << strerror(errno) << std::endl;
				break;
			}
			for (size_t i = 0; i < fds.size(); ++i)
			{
				if (fds[i].revents == 0)
					continue;

				if (fds[i].fd == server_fd)
				{
					struct sockaddr_in client_addr;
					socklen_t client_len = sizeof(client_addr);
					int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
					if (client_fd < 0)
					{
						std::cerr << "Accept failed: " << strerror(errno) << std::endl;
						continue;
					}
					struct pollfd client_pollfd;
					client_pollfd.fd = client_fd;
					client_pollfd.events = POLLIN;
					fds.push_back(client_pollfd);
					std::cout << "New client connected: " << client_fd << std::endl;
					send_message(client_fd, ":server 001 guest :Welcome to the IRC server");
				}
				else
				{
					char buffer[1024];
					int bytes_read = read(fds[i].fd, buffer, sizeof(buffer) - 1);
					if (bytes_read <= 0)
					{
						std::cout << "Client " << fds[i].fd << " disconnected" << std::endl;
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
					}
					else
					{
						buffer[bytes_read] = '\0';
						std::cout << "Received from " << fds[i].fd << ": " << buffer;
						std::string message = "Client " + std::string(buffer);
						for (size_t j = 0; j < fds.size(); ++j)
						{
							if (fds[j].fd != server_fd && fds[j].fd != fds[i].fd)
								send_message(fds[j].fd, message);
						}
					}
				}
			}
		}
	close(server_fd);
	return ;
}

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

void Server::initServer(const std::string& port)
{
	int opt;
	struct sockaddr_in address;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		std::cerr << "Error: socket creation failed " << strerror(errno) << std::endl;
		std::exit(1);
	}

	opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Error: setsockopt failed " << strerror(errno) << std::endl;
		close(server_fd);
		std::exit(1);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(std::strtol(port.c_str(), NULL, 10));

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << "Error: bind failed " << strerror(errno) << std::endl;
		close(server_fd);
		std::exit(1);
	}

	if (listen(server_fd, 10) < 0)
	{
		std::cerr << "Error: listen failed " << strerror(errno) << std::endl;
		close(server_fd);
		std::exit(1);
	}

	std::cout << BLUE"Server is running on port " << port << RESET << std::endl;

	struct pollfd server_pollfd;
	server_pollfd.fd = server_fd;
	server_pollfd.events = POLLIN;
	fds.push_back(server_pollfd);
}
