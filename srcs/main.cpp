#include "Server.hpp"

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << RED"Wrong inputs.\n" << GREEN"Usage: " << WHITE"\"" << av[0] << "\" \"port\" \"password\""RESET << std::endl;
		return (1);
	}

	Server server(av[1], av[2]);
	server.run();
	return (0);
}
