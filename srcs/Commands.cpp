#include "Commands.hpp"
#include "Parser.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <ctime>

const int Commands::BOT_FD = -1;

// When someone changes nick on the server, we need to remove the old nick from the nickList
// /JOIN asd still works, it shouldnt.
// when sending a message, chatbox doesnt appear.
// kicklendikten sonra hala mesaj atabiliyor.

static int ft_atoi(const std::string& str)
{
	std::stringstream ss(str);
	int num;
	ss >> num;
	return num;
}

std::string ft_itoa(int num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}

Commands::Commands(std::map<int, Client>& c, std::map<std::string, Channel>& ch, Server& server)
	: clients(c), channels(ch), server(server), botExists(false)
{
	initializeCommandHandlers();
}

void Commands::initializeCommandHandlers()
{
	commandHandlers["PASS"] = &Commands::handlePass;
	commandHandlers["JOIN"] = &Commands::handleJoin;
	commandHandlers["PRIVMSG"] = &Commands::handlePrivmsg;
	commandHandlers["USER"] = &Commands::handleUserCommand;
	commandHandlers["NICK"] = &Commands::handleNickCommand;
	commandHandlers["MODE"] = &Commands::handleModeCommand;
	commandHandlers["PART"] = &Commands::handlePartCommand;
	commandHandlers["QUIT"] = &Commands::handleQuitCommand;
	commandHandlers["TOPIC"] = &Commands::handleTopicCommand;
	commandHandlers["KICK"] = &Commands::handleKickCommand;
	commandHandlers["INVITE"] = &Commands::handleInviteCommand;
}

void Commands::executeCommand(const std::string& raw, Client& client)
{
	parseInfo info = Parser::parse(raw);
	std::string cmd = info.command;

	if (cmd == "PASS")
	{
		handlePass(raw, client);
		return;
	}

	if (!client.getIsAuth())
	{
		std::string err = "You have not registered yet\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (!client.isProvided())
	{
		if (cmd == "USER")
			handleUserCommand(raw, client);
		else if (cmd == "NICK")
			handleNickCommand(raw, client);
		else
		{
			std::string err = "Please provide your nickname and username by using /NICK <nickname> and /USER <'username' 0 * :'realname'>\r\n";
			send(client.getFd(), err.c_str(), err.size(), 0);
		}
		return;
	}

	std::map<std::string, CommandHandler>::iterator it = commandHandlers.find(cmd);
	if (it != commandHandlers.end())
		(this->*(it->second))(raw, client);
	else
	{
		std::string err = ":server 421 " + client.getNickname() + " " + cmd + " :Unknown command\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
	}
}

void Commands::handlePass(const std::string& message, Client& client)
{
	parseInfo info = Parser::parse(message);
	
	if (client.getIsAuth())
	{
		std::string err = "You may not reregister\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (info.function.empty())
	{
		std::string err = "Not enough parameters for PASS. Use correct format: '/PASS <pwd>'\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (info.function != client.getPwd())
	{
		std::string err = "Password is incorrect\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	client.setIsAuth(true);
	std::string success = "Welcome to the Concord. You are now registered.\r\n";
	send(client.getFd(), success.c_str(), success.size(), 0);
}

void Commands::handleUserCommand(const std::string& msg, Client& client)
{
	userInfo info = Parser::userParse(msg);
	if (info.userName.empty() || info.realName.empty())
	{
		std::string err = "Not enough parameters for USER. Use the correct format: '/USER <username> 0 * :<realname>'\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (info.userName == "bot")
	{
		std::string err = "Username 'bot' is reserved for the server bot\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (server.addUser(info.userName) == false)
	{
		std::string err = "Username already exists. Please choose a different username.\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}	

	std::string oldUsername = client.getUsername();
	if (!oldUsername.empty())
		server.removeUser(oldUsername);
	
	client.setUsername(info.userName);
	client.setRealname(info.realName);
	std::string noticeMsg = "Your username is set to: " + info.userName + "\r\n";
	send(client.getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);
	std::string noticeMsg2 = "Your realname is set to: " + info.realName + "\r\n";
	send(client.getFd(), noticeMsg2.c_str(), noticeMsg2.size(), 0);
}

void Commands::handleNickCommand(const std::string& nick, Client& client)
{
	parseInfo info = Parser::parse(nick);
	std::string cmd = info.function;
	std::string newNickname;

	if (cmd.empty())
	{
		std::string err = "No nickname given. Use the correct format: '/NICK <nickname>'\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (info.value.empty())
		newNickname = cmd;
	else
		newNickname = cmd + " " + info.value;

	if (newNickname == "IrcBot")
	{
		std::string err = "Nickname 'IrcBot' is reserved for the server bot\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (server.addNick(newNickname) == false)
	{
		std::string err = "Nickname already exists. Please choose a different nickname.\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	std::string oldNickname = client.getNickname();
	std::string msg = ":" + oldNickname + " NICK :" + newNickname + "\r\n";
	send(client.getFd(), msg.c_str(), msg.size(), 0);
	
	std::vector<std::string> channelsList = client.getJoinedChannels();
	for (std::vector<std::string>::iterator it = channelsList.begin(); it != channelsList.end(); ++it)
	{
		std::string channelName = *it;
		if (channels.find(channelName) != channels.end())
		{
			std::vector<Client>& users = channels[channelName].getUsers();
			for (std::vector<Client>::iterator userIt = users.begin(); userIt != users.end(); ++userIt)
			{
				if (userIt->getFd() != client.getFd())
				{
					std::string msgToUser = ":" + oldNickname + " NICK :" + newNickname + "\r\n";
					send(userIt->getFd(), msgToUser.c_str(), msgToUser.size(), 0);
				}
			}
			if (channels[channelName].isOp(oldNickname))
			{
				channels[channelName].removeOp(oldNickname);
				channels[channelName].addOp(newNickname);
			}
		}
	}
	
	server.removeNick(oldNickname);
	client.setNickname(newNickname);
}

bool Commands::isOP(const std::string& channelName, const Client& client)
{
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end())
	{
		const std::vector<std::string>& ops = it->second.getOps();
		return std::find(ops.begin(), ops.end(), client.getNickname()) != ops.end();
	}

	return false;
}

void Commands::handleJoin(const std::string& raw, Client& client)
{
	parseInfo info = Parser::parse(raw);
	std::string channelName = info.function;
	
	bool channelCreated = false;
	if (channels.find(channelName) == channels.end())
	{
		channels[channelName] = Channel(channelName);
		channels[channelName].setPwd("");
		channels[channelName].setName(channelName);
		channels[channelName].setInvOnly(false);
		channels[channelName].setMaxUsers(-1);
		channels[channelName].addOp(client.getNickname());
		channelCreated = true;
	}
	else
	{
		if (channels[channelName].getInvOnly())
		{
			if (std::find(channels[channelName].getInvitedUsers().begin(), channels[channelName].getInvitedUsers().end(), client.getNickname()) == channels[channelName].getInvitedUsers().end())
			{
				std::string err = ":server 473 " + client.getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n";
				send(client.getFd(), err.c_str(), err.size(), 0);
				return;
			}
			else
			{
				std::vector<std::string>::iterator toDel = std::find(channels[channelName].getInvitedUsers().begin(), channels[channelName].getInvitedUsers().end(), client.getNickname());
				channels[channelName].getInvitedUsers().erase(toDel);
			}
		}

		if (channels[channelName].getPwd() != "" && info.value != channels[channelName].getPwd())
		{
			std::string err = ":server 475 " + client.getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n";
			send(client.getFd(), err.c_str(), err.size(), 0);
			return;
		}

		if (channels[channelName].getMaxUsers() != -1 && 
			static_cast<int>(channels[channelName].getUsers().size()) >= channels[channelName].getMaxUsers())
		{
			std::string err = ":server 471 " + client.getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n";
			send(client.getFd(), err.c_str(), err.size(), 0);
			return;
		}

		std::vector<Client>::iterator it;
		std::vector<Client>& users = channels[channelName].getUsers();
		
		for (it = users.begin(); it != users.end(); ++it)
		{
			if (it->getNickname() == client.getNickname())
			{
				std::string err = ":server 443 " + client.getNickname() + " " + channelName + " :is already on channel\r\n";
				send(client.getFd(), err.c_str(), err.size(), 0);
				return;
			}
		}
	}

	channels[channelName].addUser(client);
	client.joinChannel(channelName);

	std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + client.getHostname() + " JOIN :" + channelName + "\r\n";
	for (std::vector<Client>::iterator it = channels[channelName].getUsers().begin(); it != channels[channelName].getUsers().end(); ++it)
		send(it->getFd(), joinMsg.c_str(), joinMsg.length(), 0);

	std::string topicMsg = ":server 332 " + client.getNickname() + " " + channelName + " :" + channels[channelName].getTopic() + "\r\n";
	send(client.getFd(), topicMsg.c_str(), topicMsg.length(), 0);

	std::string names;

	std::vector<Client> &users = channels[channelName].getUsers();
	for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (channels[channelName].isOp(it->getNickname()))
			names += "@" + it->getNickname() + " ";
		else
			names += it->getNickname() + " ";
	}

	std::string namesMsg = ":server 353 " + client.getNickname() + " = " + channelName + " :" + names + "\r\n";
	send(client.getFd(), namesMsg.c_str(), namesMsg.length(), 0);

	std::string endNames = ":server 366 " + client.getNickname() + " " + channelName + " :End of /NAMES list.\r\n";
	send(client.getFd(), endNames.c_str(), endNames.length(), 0);
	
	if (channelCreated)
	{
		std::string modeMsg = ":" + client.getNickname() + " MODE " + channelName + " +o " + client.getNickname() + "\r\n";
		std::vector<Client>& users = channels[channelName].getUsers();
		for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
			send(it->getFd(), modeMsg.c_str(), modeMsg.length(), 0);
		botJoinChannel(channelName);
	}
	else
		botJoinChannel(channelName);

	botGreetUser(channelName, client.getNickname());
	botGiveOpToUser(channelName, client.getNickname());
}

void Commands::handlePartCommand(const std::string& msg, Client& client)
{
	std::vector<std::string> words = split(msg);
	if (words.size() < 2)
	{
		std::string err = ":server 461 " + client.getNickname() + " :Not enough parameters\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	std::string channelName = words[1];
	if (channels.find(channelName) == channels.end())
	{
		std::string err = ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	std::string noticeMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " PART " + channelName + "\r\n";
	for (std::vector<Client>::iterator it = channels[channelName].getUsers().begin(); it != channels[channelName].getUsers().end(); ++it)
		send(it->getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);
	channels[channelName].removeUser(client);
	channels[channelName].removeOp(client.getNickname());
	client.partChannel(channelName);
	if (channels[channelName].getUsers().empty())
		channels.erase(channelName);
}

void Commands::handleTopicCommand(const std::string& msg, Client& client)
{
	std::vector<std::string> words = split(msg);
	if (words.size() < 2)
	{
		std::string err = ":server 461 " + client.getNickname() + " " + words[0] + " :Not enough parameters\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	std::string channelName = words[1];
	if (channels.find(channelName) == channels.end())
	{
		std::string err = ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (words.size() != 2 && !isOP(channelName, client) && !channels[channelName].getTopicSet())
	{
		std::string err = ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (words.size() == 2)
	{
		std::string topic = channels[channelName].getTopic();
		std::string topicMsg = ":server 332 " + client.getNickname() + " " + channelName + " :" + topic + "\r\n";
		send(client.getFd(), topicMsg.c_str(), topicMsg.size(), 0);
		return;
	}
	else
	{
		std::string topicHandled;
		if (words[2][0] != ':')
		{
			std::string err = ":server 461 " + client.getNickname() + " " + words[0] + " :Not enough parameters\r\n";
			send(client.getFd(), err.c_str(), err.size(), 0);
			return;
		}
		words[2].erase(0, 1);
		for(size_t i = 2; i < words.size(); ++i)
		{
			if (i > 2)
				topicHandled += " ";
			topicHandled += words[i];
		}
		channels[channelName].setTopic(topicHandled);
	}

	std::string topic = channels[channelName].getTopic();
	std::string topicSetBy = client.getNickname();
	std::time_t topicSetAt = std::time(NULL);

	std::string topicMsg = ":server 332 " + client.getNickname() + " " + channelName + " :" + topic + "\r\n";
	std::string topicSetMsg = ":server 333 " + client.getNickname() + " " + channelName + " " + topicSetBy + " " + ft_itoa(topicSetAt) + "\r\n";

	std::vector<Client>& users = channels[channelName].getUsers();
	for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
	{
		send(it->getFd(), topicMsg.c_str(), topicMsg.size(), 0);
		send(it->getFd(), topicSetMsg.c_str(), topicSetMsg.size(), 0);
	}
}

void Commands::handleModeCommand(const std::string& msg, Client& client)
{
	modeInfo info = Parser::modeParse(msg);
	if (info.key.empty())
	{
		std::string modes = "";
		if (channels[info.channel].getInvOnly())
			modes += "i";
		if (!channels[info.channel].getPwd().empty())
			modes += "k";
		if (channels[info.channel].getMaxUsers() != -1)
			modes += "l";
		if (channels[info.channel].getTopicSet())
			modes += "t";
		if (modes.empty())
		{
			modes = ":" + client.getNickname() + " NOTICE " + client.getNickname() + " Current modes in " + info.channel + " are: None" + "\r\n";
			send(client.getFd(), modes.c_str(), modes.size(), 0);
			return;
		}
		std::string noticeMsg = ":" + client.getNickname() + " NOTICE " + client.getNickname() + " Current modes in " + info.channel + " are: +" + modes;
		noticeMsg += "\r\n";
		send(client.getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);
		return;
	}

	if (isOP(channels[info.channel].getName(), client) == false)
	{
		std::string err = ":server 482 " + client.getNickname() + " " + channels[info.channel].getName() + " :Permission Denied - You're not an operator in this server.\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	std::vector<std::string> &ops = channels[info.channel].getOps();
	std::vector<std::string>::iterator it = ops.begin();
	std::vector<std::string>::iterator ite = ops.end();
	std::string noticeMsg;
	while (it != ite)
	{
		if (*it == client.getNickname())
		{
			if (info.key == "i")
			{
				if (!channels[info.channel].getInvOnly())
					channels[info.channel].setInvOnly(info.status);
				else
					channels[info.channel].setInvOnly(0);
				noticeMsg = ":" + client.getNickname() + " MODE " + info.channel + " " + (info.status ? "+i" : "-i") + "\r\n";
			}
			else if (info.key == "k")
			{
				if (channels[info.channel].getPwd().empty() || !info.status)
					channels[info.channel].setPwd("");
				else
					channels[info.channel].setPwd(info.parameters);
				noticeMsg = ":" + client.getNickname() + " MODE " + info.channel + " " + (info.status ? "+k" : "-k") + " " + info.parameters + "\r\n";
			}
			else if (info.key == "l")
			{
				int maxUsers = -1;
				if (!info.status)
					channels[info.channel].setMaxUsers(maxUsers);
				else
				{
					maxUsers = ft_atoi(info.parameters);
					channels[info.channel].setMaxUsers(maxUsers);
				}
				noticeMsg = ":" + client.getNickname() + " MODE " + info.channel + " " + (info.status ? "+l" : "-l") + " " + (maxUsers == -1 ? "" : ft_itoa(maxUsers)) + "\r\n";
			}
			else if (info.key == "o")
			{
				if (channels[info.channel].isUserInChannel(info.parameters) == false)
				{
					std::string err = ":" + client.getNickname() +" NOTICE " + client.getNickname() + " :No such user in the Channel\r\n";
					send(client.getFd(), err.c_str(), err.size(), 0);
					return;
				}

				if (!info.status && info.parameters == "IrcBot")
				{
					std::string err = ":" + client.getNickname() +" NOTICE " + client.getNickname() + " :IrcBot cannot be deop'd\r\n";
					send(client.getFd(), err.c_str(), err.size(), 0);
					return;
				}

				if (info.status)
				{
					if (std::find(channels[info.channel].getOps().begin(), channels[info.channel].getOps().end(), info.parameters) == channels[info.channel].getOps().end())
						channels[info.channel].addOp(info.parameters);
					else
					{
						std::string err = ":" + client.getNickname() +" NOTICE " + client.getNickname() + " :The user is already OP!\r\n";
						send(client.getFd(), err.c_str(), err.size(), 0);
						return;
					}
				}
				else
				{
					if (std::find(channels[info.channel].getOps().begin(), channels[info.channel].getOps().end(), info.parameters) != channels[info.channel].getOps().end())
						channels[info.channel].removeOp(info.parameters);
					else
					{
						std::string err = ":" + client.getNickname() +" NOTICE " + client.getNickname() + " :The user is not OP!\r\n";
						send(client.getFd(), err.c_str(), err.size(), 0);
						return;
					}
				}

				noticeMsg = ":" + client.getNickname() + " MODE " + info.channel + " " + (info.status ? "+o" : "-o") + " " + info.parameters + "\r\n";
			}
			else if (info.key == "t")
			{
				if (info.status)
					channels[info.channel].setTopicSet(true);
				else
					channels[info.channel].setTopicSet(false);

				noticeMsg = ":" + client.getNickname() + " MODE " + info.channel + " " + (info.status ? "+t" : "-t") + "\r\n";
			}
			else
			{
				std::string err = ":server PRIVMSG " + client.getNickname() + " " + info.key + " :is unknown mode char to me\r\n";
				send(client.getFd(), err.c_str(), err.size(), 0);
				return;
			}

			for (std::vector<Client>::iterator user = channels[info.channel].getUsers().begin(); user != channels[info.channel].getUsers().end(); ++user)
				send(user->getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);

			return;
		}
		++it;
	}

	std::string err = "482 " + client.getNickname() + " " + info.channel + " :You're not channel operator\r\n";
	send(client.getFd(), err.c_str(), err.size(), 0);
}

void Commands::handlePrivmsg(const std::string& message, Client& sender)
{
	reciveMessage info = Parser::privateMessage(message);
	if (info.target.empty() || info.message.empty())
	{
		std::string err = "411 " + sender.getNickname() + " :No recipient given\r\n";
		send(sender.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.getNickname() == info.target)
		{
			std::string msg = ":" + sender.getNickname() + " PRIVMSG " + info.target + " :" + info.message + "\r\n";
			send(it->first, msg.c_str(), msg.size(), 0);
			return;
		}
	}

	std::map<std::string, Channel>::iterator it = channels.find(info.target);
	if (it != channels.end())
	{
		std::vector<Client>& users = it->second.getUsers();
		std::string msg = ":" + sender.getNickname() + "!" + sender.getUsername() + "@" + sender.getHostname() + " PRIVMSG " + info.target + " :" + info.message + "\r\n";
		for (std::vector<Client>::iterator user = users.begin(); user != users.end(); ++user)
		{
			if (user->getFd() != sender.getFd())
				send(user->getFd(), msg.c_str(), msg.size(), 0);
		}
		return;
	}

	std::string err = "401 " + sender.getNickname() + " " + info.target + " :No such nick/channel\r\n";
	send(sender.getFd(), err.c_str(), err.size(), 0);
}

void Commands::handleKickCommand(const std::string& msg, Client& client)
{
	std::vector<std::string> words = split(msg);
	if (words.size() < 3)
	{
		std::string err = "461 " + client.getNickname() + " :Not enough parameters\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	std::string channelName = words[1];
	std::string targetNick = words[2];
	if (channels.find(channelName) == channels.end())
	{
		std::string err = "403 " + client.getNickname() + " " + channelName + " :No such channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	std::vector<Client>& users = channels[channelName].getUsers();
	bool isKickerInChannel = false;

	for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->getNickname() == client.getNickname())
		{
			isKickerInChannel = true;
			break;
		}
	}

	if (!isKickerInChannel)
	{
		std::string err = "442 " + channelName + " :You're not on that channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (client.getNickname() == targetNick)
	{
		std::string err = ":" + client.getNickname() + " NOTICE " + client.getNickname() + " :Can't kick yourself\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (!isOP(channelName, client))
	{
		std::string err = "482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (targetNick == "IrcBot")
	{
		std::string err = ":" + client.getNickname() + " NOTICE " + client.getNickname() + " :IrcBot cannot be kicked\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->getNickname() == targetNick)
		{
			std::string noticeMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " KICK " + channelName + " " + targetNick + "\r\n";
			for (std::vector<Client>::iterator it = channels[channelName].getUsers().begin(); it != channels[channelName].getUsers().end(); ++it)
				send(it->getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);

			for (std::map<int, Client>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
			{
				if (clientIt->second.getNickname() == targetNick)
				{
					clientIt->second.partChannel(channelName);
					break;
				}
			}
			
			channels[channelName].removeUser(*it);
			channels[channelName].removeOp(targetNick);
			return;
		}
	}
	std::string err = ":" + client.getNickname() + " NOTICE " + client.getNickname() + " " + targetNick + " is not in that channel\r\n";
	send(client.getFd(), err.c_str(), err.size(), 0);
}

void Commands::handleInviteCommand(const std::string& msg, Client& client)
{
	std::vector<std::string> words = split(msg);
	if (words.size() < 3)
	{
		std::string err = "461 " + client.getNickname() + " :Not enough parameters\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	std::string targetNick = words[1];
	std::string channelName = words[2];
	if (channels.find(channelName) == channels.end())
	{
		std::string err = "403 " + client.getNickname() + " " + channelName + " :No such channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	if (!isOP(channelName, client))
	{
		std::string err = "482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}

	channels[channelName].addinvitedUser(targetNick);
	std::string inviteMsg = ":" + client.getNickname() + " INVITE " + targetNick + " :" + channelName + "\r\n";
	send(client.getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);
	std::string noticeMsg = ":server NOTICE " + targetNick + " :You have been invited to join " + channelName + "\r\n";

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.getNickname() == targetNick)
		{
			send(it->first, noticeMsg.c_str(), noticeMsg.size(), 0);
			return;
		}
	}
}

void Commands::handleQuitCommand(const std::string& msg, Client& client)
{
	std::string quitMsg = ":" + client.getNickname() + " QUIT :" + msg + "\r\n";
	send(client.getFd(), quitMsg.c_str(), quitMsg.size(), 0);
	if (!client.getJoinedChannels().empty())
	{
		std::vector<std::string> channelsList = client.getJoinedChannels();
		std::vector<std::string>::iterator it = channelsList.begin();
		std::vector<std::string>::iterator ite = channelsList.end();
		while (it != ite)
		{
			std::string channelName = *it;
			if (channels.find(channelName) != channels.end())
			{
				std::vector<Client>& users = channels[channelName].getUsers();
				std::vector<Client>::iterator userIt = users.begin();
				std::vector<Client>::iterator userIte = users.end();
				while (userIt != userIte)
				{
					if (userIt->getFd() != client.getFd())
						send(userIt->getFd(), quitMsg.c_str(), quitMsg.size(), 0);
					userIt++;
				}
				channels[channelName].removeUser(client);
				channels[channelName].removeOp(client.getNickname());
				if (channels[channelName].getUsers().empty())
					channels.erase(channelName);
			}
			it++;
		}
	}
	client.clearBuffer();
	clients.erase(client.getFd());
}

void Commands::createBot()
{
	if (botExists)
		return;
		
	Client bot(BOT_FD);
	bot.setNickname("IrcBot");
	bot.setUsername("bot");
	bot.setRealname("IRC Helper Bot");
	bot.setHostname("server");
	bot.setIsAuth(true);

	clients.insert(std::make_pair(BOT_FD, bot));

	std::string botNick = "IrcBot";
	std::string botUser = "bot";
	server.addNick(botNick);
	server.addUser(botUser);

	botExists = true;
}

void Commands::botJoinChannel(const std::string& channelName)
{
	if (!botExists)
		createBot();
		
	std::map<int, Client>::iterator botIt = clients.find(BOT_FD);
	if (botIt == clients.end())
		return;

	Client& bot = botIt->second;

	if (channels.find(channelName) != channels.end())
	{

		std::vector<Client>& users = channels[channelName].getUsers();
		for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (it->getFd() == BOT_FD)
				return;
		}
		
		channels[channelName].addUser(bot);
		channels[channelName].addOp("IrcBot");
		bot.joinChannel(channelName);
		
		std::string joinMsg = ":IrcBot!bot@serverl JOIN :" + channelName + "\r\n";
		for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (it->getFd() != BOT_FD && it->getFd() != -1)
				send(it->getFd(), joinMsg.c_str(), joinMsg.length(), 0);
		}
		
		std::string modeMsg = ":IrcBot MODE " + channelName + " +o IrcBot\r\n";
		for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (it->getFd() != BOT_FD && it->getFd() != -1)
				send(it->getFd(), modeMsg.c_str(), modeMsg.length(), 0);
		}
	}
}

void Commands::botGreetUser(const std::string& channelName, const std::string& nickname)
{
	if (!botExists || channels.find(channelName) == channels.end())
		return;

	std::vector<Client>& users = channels[channelName].getUsers();
	for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->getNickname() == nickname && it->getFd() != BOT_FD)
		{
			std::string greetMsg = ":IrcBot!bot@server PRIVMSG " + channelName + " :Welcome to " + channelName + ", " + nickname + "!\r\n";
			for (std::vector<Client>::iterator userIt = users.begin(); userIt != users.end(); ++userIt)
			{
				if (userIt->getFd() != BOT_FD && userIt->getFd() != -1)
					send(userIt->getFd(), greetMsg.c_str(), greetMsg.length(), 0);
			}
			break;
		}
	}
}

void Commands::botGiveOpToUser(const std::string& channelName, const std::string& nickname)
{
	if (!botExists || channels.find(channelName) == channels.end())
		return;
	bool shouldGiveOP = false;
	if (nickname == "abakirca" || nickname == "Myxoceph")
		shouldGiveOP = true;
	
	if (shouldGiveOP)
	{
		if (channels[channelName].isOp(nickname))
			return;

		channels[channelName].addOp(nickname);
		
		std::string modeMsg = ":IrcBot MODE " + channelName + " +o " + nickname + "\r\n";

		std::vector<Client>& users = channels[channelName].getUsers();
		for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (it->getFd() != BOT_FD && it->getFd() != -1)
				send(it->getFd(), modeMsg.c_str(), modeMsg.length(), 0);
		}

		for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (it->getNickname() == nickname && it->getFd() != -1)
			{
				std::string privMsg = ":IrcBot!bot@server PRIVMSG " + nickname + " :Welcome, Ahmet. I have granted you the operator privileges.\r\n";
				send(it->getFd(), privMsg.c_str(), privMsg.length(), 0);
				break;
			}
		}
	}
}

bool Commands::isBotCreated() const
{
	return botExists;
}
