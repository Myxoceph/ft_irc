#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <map>
#include <string>

class Client;
class Channel;

class Commands {
private:
	std::map<int, Client>& clients;
	std::map<std::string, Channel>& channels;

public:
	Commands(std::map<int, Client>& c, std::map<std::string, Channel>& ch);
	void executeCommand(const std::string& raw, Client& client);

private:
	void handleJoin(const std::string& channelName, Client& client);
	void handlePrivmsg(const std::string& message, Client& sender);
	void handleUserCommand(const std::string& msg, Client& client);
	void handleNickCommand(const std::string& nick, Client& client);
	void handleModeCommand(const std::string& msg, Client& client);
	void handlePartCommand(const std::string& msg, Client& client);
	void handleQuitCommand(const std::string& msg, Client& client);
	void handleTopicCommand(const std::string& msg, Client& client);
	void handleKickCommand(const std::string& msg, Client& client);
	void handleInviteCommand(const std::string& msg, Client& client);
	bool isOP(const std::string& channelName, const Client& client);
};

#endif
