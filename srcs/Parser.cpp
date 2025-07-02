#include "Parser.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace {
	// Helper function to split a string by a delimiter.
	// Note: This is a simple implementation and may not handle all edge cases perfectly (e.g., multiple spaces).
	std::vector<std::string> split(const std::string& str, const std::string& delimiter)
	{
		std::vector<std::string> tokens;
		size_t pos = 0;
		std::string token;
		std::string s = str;

		while ((pos = s.find(delimiter)) != std::string::npos) {
			token = s.substr(0, pos);
			if (!token.empty())
				tokens.push_back(token);
			s.erase(0, pos + delimiter.length());
		}
		if (!s.empty())
			tokens.push_back(s);
		return tokens;
	}
} // anonymous namespace

// Parses a raw IRC message into a command and its parameters.
parseInfo Parser::parse(std::string message)
{
	parseInfo info;
	std::vector<std::string> tokens;

	// Check for a trailing parameter (prefixed with a colon)
	size_t colonPos = message.find(" :");

	if (colonPos != std::string::npos)
	{
		std::string beforeColon = message.substr(0, colonPos);
		std::string afterColon = message.substr(colonPos + 2);
		tokens = split(beforeColon, " ");
		tokens.push_back(afterColon);
	}
	else
	{
		tokens = split(message, " ");
	}

	// The first token is the command, the rest are parameters.
	if (!tokens.empty())
	{
		info.command = tokens[0];
		tokens.erase(tokens.begin());
		info.params = tokens;
	}

	return info;
}
