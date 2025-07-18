#include "Parser.hpp"
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& inputStr) {
	std::vector<std::string> result;
	size_t i = 0;

	while (i < inputStr.length()) {
		while (i < inputStr.length() && std::isspace(inputStr[i])) ++i;
		size_t begin = i;
		while (i < inputStr.length() && !std::isspace(inputStr[i])) ++i;
		if (begin < i)
			result.push_back(inputStr.substr(begin, i - begin));
	}
	return result;
}

std::string trim(const std::string& raw) {
	size_t left = 0;
	while (left < raw.size() && std::isspace(raw[left])) ++left;

	if (left == raw.size())
		return "";

	size_t right = raw.size() - 1;
	while (right > left && std::isspace(raw[right])) --right;

	return raw.substr(left, right - left + 1);
}

reciveMessage Parser::privateMessage(std::string rawMsg) {
	reciveMessage data;
	std::vector<std::string> parts = split(rawMsg);

	if (parts.size() < 3)
		return data;

	data.target = parts[1];

	if (!parts[2].empty() && parts[2][0] == ':')
		parts[2].erase(0, 1);

	for (size_t i = 2; i < parts.size(); i++) {
		data.message += trim(parts[i]);
		if (i + 1 < parts.size()) {
			data.message += " ";
		}
	}
	return data;
}

parseInfo Parser::parse(std::string rawInput) {
	parseInfo parsed;

	std::vector<std::string> segments = split(rawInput);
	if (segments.empty()) {
		parsed.command = rawInput;
		return parsed;
	}
	
	parsed.command = trim(segments[0]);

	if (segments.size() > 1) {
		parsed.function = trim(segments[1]);
	}
	
	for (size_t i = 2; i < segments.size(); i++) {
		parsed.value += trim(segments[i]);
		if (i + 1 < segments.size()) {
			parsed.value += " ";
		}
	}
	return parsed;
}

userInfo Parser::userParse(std::string rawUserInput) {
	userInfo result;
	std::vector<std::string> fields = split(rawUserInput);

	if (fields.size() < 5 || fields[4].empty() || fields[4][0] != ':')
		return result;

	result.userName = fields[1];
	result.realName = fields[4].substr(1);
	return result;
}

modeInfo Parser::modeParse(std::string modeLine)
{
	modeInfo inf;
	std::vector<std::string> args = split(modeLine);

	if (args.size() == 2){
		inf.channel = trim(args[1]);
		return inf;
	}

	if (args.size() < 3)
		return inf;

	inf.channel = trim(args[1]);
	inf.status = (args[2][0] == '+');
	inf.key = (args[2].size() > 1) ? args[2][1] : '\0';
	
	if (args.size() == 4) {
		inf.parameters = trim(args[3]);
	}
	if (args.size() == 5) {
		inf.parameters += trim(args[4]);
	}
	return inf;
}