#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

struct userInfo {
	std::string userName;
	std::string realName;
};

struct reciveMessage {
	std::string target;
	std::string message;
};

struct parseInfo {
	std::string command;
	std::string function;
	std::string value;
};

struct modeInfo {
	std::string channel;
	bool status;
	std::string key;
	std::string parameters;
};

class Parser {
	public:
		static reciveMessage privateMessage(std::string rawMsg);
		static parseInfo parse(std::string rawInput);
		static userInfo userParse(std::string rawUserInput);
		static modeInfo modeParse(std::string modeLine);
};

std::vector<std::string> split(const std::string& inputStr);
std::string trim(const std::string& raw);

#endif
