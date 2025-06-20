#include "Server.hpp"

int main(int ac, char **av)
{
	try
	{
		if (ac != 3)
			throw std::invalid_argument(RED"Wrong inputs.\n" GREEN"Usage: " WHITE"\"" + std::string(av[0]) + "\" \"port\" \"password\"" RESET);
		Server server(av[1], av[2]);
		server.run();
		return (0);
	}
	catch(const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}
}
