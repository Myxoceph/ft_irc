#include "Parser.hpp"
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& s)
{
	std::vector<std::string> tokens;

	size_t start = 0, end = 0;
	while (start < s.length())
	{
		while (start < s.length() && std::isspace(s[start])) ++start;
		if (start >= s.length()) break;

		end = start;
		while (end < s.length() && !std::isspace(s[end])) ++end;

		tokens.push_back(s.substr(start, end - start));
		start = end;
	}
	return tokens;
}

std::string trim(const std::string& str)
{
	size_t start = 0;
	size_t end = str.size() - 1;

	while (start <= end && std::isspace(str[start]))
		++start;

	while (end >= start && std::isspace(str[end]))
		--end;

	return str.substr(start, end - start + 1);
}

reciveMessage Parser::privateMessage(std::string message)
{
	reciveMessage info;

	std::vector<std::string> words = split(message);
	if (words.size() < 3)
	{
		info.target = "";
		info.message = "";
		return info;
	}

	info.target = words[1];

	if (words[2][0] == ':')
		words[2].erase(0, 1);

	for (size_t i = 2; i < words.size(); i++)
	{
		info.message += trim(words[i]);
		if (i != words.size() - 1)
			info.message += " ";
	}

	return info;
}

parseInfo Parser::parse(std::string message)
{
	parseInfo info;

	if (message.find(" ") == std::string::npos)
	{
		info.command = message;
		return info;
	}

	std::vector<std::string> words = split(message);

	info.command = trim(words[0]);
	if (words.size() == 1)
		return info;

	info.function = trim(words[1]);
	if (words.size() == 2)
	{
		info.value = "";
		return info;
	}

	for (size_t i = 2; i < words.size(); i++)
	{
		info.value += trim(words[i]);
		if (i != words.size() - 1)
			info.value += " ";
	}

	return info;
}

userInfo Parser::userParse(std::string message)
{
	userInfo user;
	std::string realName;

	std::vector<std::string> words = split(message);
	if (words.size() < 5 || words[4][0] != ':')
	{
		user.userName = "";
		user.realName = "";
		return user;
	}

	user.userName = words[1];
	realName = words[4].replace(0, 1, "");

	if (words.size() == 5)
		user.realName = realName;
	else
	{
		for (size_t i = 5; i < words.size(); i++)
			realName += " " + words[i];
		user.realName = realName;
	}

	return user;
}

modeInfo Parser::modeParse(std::string message)
{
	modeInfo info;

	std::vector<std::string> words = split(message);
	if (words.size() < 2)
	{
		info.channel = "";
		info.status = false;
		info.key = "";
		info.parameters = "";
		return info;
	}
	if (words.size() == 2)
	{
		info.channel = trim(words[1]);
		info.status = false;
		info.key = "";
		info.parameters = "";
		return info;
	}

	if (words.size() < 3)
	{
		info.channel = "";
		info.status = false;
		info.key = "";
		info.parameters = "";
		return info;
	}

	info.channel = trim(words[1]);
	info.status = words[2][0] == '+';
	info.key = words[2][1];

	if (words.size() == 3) 
		return info;

	info.parameters = trim(words[3]);
	if (words.size() == 4)
		return info;

	if (words.size() == 5)
		info.parameters += trim(words[4]);

	return info;
}
