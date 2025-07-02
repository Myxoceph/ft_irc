#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

// Structure to hold parsed IRC message info
struct parseInfo
{
	std::string command;
	std::vector<std::string> params;
};

// Parser class for processing raw IRC messages
class Parser
{
	public:
		static parseInfo parse(std::string message);
};

#endif
