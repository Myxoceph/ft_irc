#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include "Parser.hpp"
#include <map>
#include <string>

class Client;
class Channel;
struct parseInfo;

class Commands {
public:
	// Constructor
	Commands(std::map<int, Client>& c, std::map<std::string, Channel>& ch);

	// Public Methods
	bool executeCommand(const std::string& raw, Client& client);

private:
	// Typedefs
	typedef void (Commands::*CommandHandler)(const parseInfo&, Client&);

	// Private Members
	std::map<std::string, CommandHandler> _commandHandlers;
	std::map<int, Client>& _clients;
	std::map<std::string, Channel>& _channels;

	// Command Handlers
	void handlePass(const parseInfo& info, Client& client);
	void handleNick(const parseInfo& info, Client& client);
	void handleUser(const parseInfo& info, Client& client);
	void handleJoin(const parseInfo& info, Client& client);
	void handlePrivmsg(const parseInfo& info, Client& client);
	void handleMode(const parseInfo& info, Client& client);
	void handlePart(const parseInfo& info, Client& client);
	void handleQuit(const parseInfo& info, Client& client);
	void handleTopic(const parseInfo& info, Client& client);
	void handleKick(const parseInfo& info, Client& client);
	void handleInvite(const parseInfo& info, Client& client);

	// Helper Functions
	void initializeCommands();
	void sendReply(const Client& client, const std::string& reply);
	void broadcast(const std::string& channelName, const std::string& message, bool excludeSender, const Client& sender);
};

#endif
