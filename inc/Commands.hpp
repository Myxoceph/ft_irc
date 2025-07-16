#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include <map>
#include <string>

class Client;
class Channel;
class Server;

class Commands
{
	private:
		std::map<int, Client>& clients;
		std::map<std::string, Channel>& channels;
		static const int BOT_FD; // set -1 to indicate bot because it is not a real client
		Server& server;
		bool botExists;

		typedef void (Commands::*CommandHandler)(const std::string&, Client&);

		std::map<std::string, CommandHandler> commandHandlers;
		void initializeCommandHandlers();

		void handlePass(const std::string& message, Client& client);
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

		void createBot();
		void botJoinChannel(const std::string& channelName);
		void botGreetUser(const std::string& channelName, const std::string& nickname);
		void botGiveOpToUser(const std::string& channelName, const std::string& nickname);
		bool isBotCreated() const;

		public:
			Commands(std::map<int, Client>& c, std::map<std::string, Channel>& ch, Server& server);
			void executeCommand(const std::string& raw, Client& client);
};

std::string ft_itoa(int num);

#endif
