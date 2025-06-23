#include "Commands.hpp"
#include "Parser.hpp"
#include <iostream>

Commands::Commands(std::map<int, Client>& c, std::map<std::string, Channel>& ch)
	: clients(c), channels(ch) {}

void Commands::executeCommand(const std::string& raw, Client& client) {
	parseInfo info = Parser::parse(raw);
	std::string cmd = info.command;

	if (cmd == "JOIN")
		handleJoin(info.function, client);
	else if (cmd == "PRIVMSG")
		handlePrivmsg(raw, client);
	else if (cmd == "USER")
		handleUserCommand(raw, client);
	else if (cmd == "NICK")
		handleNickCommand(info.function, client);
	else if (cmd == "MODE")
		handleModeCommand(raw, client);
	else
		std::cout << "Unknown command: " << cmd << std::endl;
}

void Commands::handleJoin(const std::string& channelName, Client& client)
{
	if (channels.find(channelName) == channels.end()) {
		channels[channelName] = Channel(channelName);
		std::cout << "New channel created: " << channelName << std::endl;
	}
	channels[channelName].addUser(client);
}

void Commands::handlePrivmsg(const std::string& message, Client& sender)
{
	reciveMessage info = Parser::privateMessage(message);

	if (info.target.empty() || info.message.empty()) {
		std::string err = "411 " + sender.getNickname() + " :No recipient given\r\n";
		send(sender.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second.getNickname() == info.target) {
			std::string msg = ":" + sender.getNickname() + " PRIVMSG " + info.target + " :" + info.message + "\r\n";
			send(it->first, msg.c_str(), msg.size(), 0);
			return;
		}
	}
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		if (it->second.getName() == info.target)
		{
			std::string msg = ":" + sender.getNickname() + " PRIVMSG " + info.target + " :" + info.message + "\r\n";
			std::vector<Client>& users = it->second.getUsers();
			for (std::vector<Client>::iterator user = users.begin(); user != users.end(); ++user) {
				send(user->getFd(), msg.c_str(), msg.size(), 0);
			}
			return;
		}
	}
	std::string err = "401 " + sender.getNickname() + " " + info.target + " :No such nick/channel\r\n";
	send(sender.getFd(), err.c_str(), err.size(), 0);
}

void Commands::handleUserCommand(const std::string& msg, Client& client)
{

	userInfo info = Parser::userParse(msg);
	if (info.nickName.empty() || info.userName.empty() || info.realName.empty()) {
		std::string err = "461 " + client.getNickname() + " :Not enough parameters\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	client.setUsername(info.userName);
	client.setRealname(info.realName);
}

void Commands::handleNickCommand(const std::string& nick, Client& client)
{
	if (nick.empty())
	{
		std::string err = "431 :No nickname given\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	client.setNickname(nick);
	std::cout << "Nickname set to: " << nick << std::endl;
}

void Commands::handleModeCommand(const std::string& msg, Client& client)
{
	modeInfo info = Parser::modeParse(msg);
	if (info.channel.empty())
	{
		std::string err = "401 " + client.getNickname() + " :No such channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	std::cout << "Mode change on channel: " << info.channel << " -> " << (info.status ? "+" : "-") << info.key << std::endl;
}
