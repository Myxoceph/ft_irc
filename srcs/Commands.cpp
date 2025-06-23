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

void Commands::handleJoin(const std::string& channelName, Client& client) {
	if (channels.find(channelName) == channels.end()) {
		channels[channelName] = Channel(channelName);
		std::cout << "New channel created: " << channelName << std::endl;
	}
	std::cout << client.getNickname() << " joined " << channelName << std::endl;
}

void Commands::handlePrivmsg(const std::string& message, Client& sender) {
	reciveMessage info = Parser::privateMessage(message);

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second.getNickname() == info.target) {
			std::string msg = ":" + sender.getNickname() + " PRIVMSG " + info.target + " :" + info.message + "\r\n";
			send(it->first, msg.c_str(), msg.size(), 0);
			return;
		}
	}
	std::string err = "401 " + sender.getNickname() + " " + info.target + " :No such nick/channel\r\n";
	send(sender.getFd(), err.c_str(), err.size(), 0);
}

void Commands::handleUserCommand(const std::string& msg, Client& client) {
	userInfo info = Parser::userParse(msg);
	client.setUsername(info.userName);
	client.setRealname(info.realName);
}

void Commands::handleNickCommand(const std::string& nick, Client& client) {
	client.setNickname(nick);
}

void Commands::handleModeCommand(const std::string& msg, Client& client) {
	modeInfo info = Parser::modeParse(msg);
	std::cout << "Mode change on channel: " << info.channel
	          << " -> " << (info.status ? "+" : "-") << info.key << std::endl;
}
