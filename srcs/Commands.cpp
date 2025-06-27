#include "Commands.hpp"
#include "Parser.hpp"
#include <iostream>

static int ft_atoi(const std::string& str)
{
	std::stringstream ss(str);
	int num;
	ss >> num;
	return num;
}

static std::string ft_itoa(int num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}

Commands::Commands(std::map<int, Client>& c, std::map<std::string, Channel>& ch)
	: clients(c), channels(ch) {}

void Commands::executeCommand(const std::string& raw, Client& client)
{
	parseInfo info = Parser::parse(raw);
	std::string cmd = info.command;

	if (cmd == "PASS")
	{
		if (client.getIsAuth())
		{
			std::string err = "462 " + client.getNickname() + " :You may not reregister\r\n";
			send(client.getFd(), err.c_str(), err.size(), 0);
			return;
		}
		if (info.function.empty())
		{
			std::string err = "461 " + client.getNickname() + " PASS :Not enough parameters\r\n";
			send(client.getFd(), err.c_str(), err.size(), 0);
			return;
		}
		if (info.function != client.getPwd())
		{
			std::string err = "464 :Password incorrect. Use /PASS <pwd> and provide the password\r\n";
			send(client.getFd(), err.c_str(), err.size(), 0);
			return;
		}
		client.setIsAuth(true);
		std::string success = "Welcome to the Concord. You are now registered.\r\n";
		send(client.getFd(), success.c_str(), success.size(), 0);
		return;
	}
	else if (client.getIsAuth())
	{
		if (!client.isProvided())
		{
			if (cmd != "USER" && cmd != "NICK")
			{
				std::string err = "Please provide your nickname and/or username by using /NICK <nickname> and/or /USER <'username' 0 * :'realname'>\r\n";
				send(client.getFd(), err.c_str(), err.size(), 0);
				return;
			}
			else if (cmd == "USER")
				handleUserCommand(raw, client);
			else if (cmd == "NICK")
				handleNickCommand(info.function, client);
		}
		else if (client.isProvided())
		{
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
			else if (cmd == "PART")
				handlePartCommand(raw, client);
			else if (cmd == "QUIT")
				handleQuitCommand(raw, client);
			else if (cmd == "TOPIC")
				handleTopicCommand(raw, client);
			else if (cmd == "KICK")
				handleKickCommand(raw, client);
			else if (cmd == "INVITE")
				handleInviteCommand(raw, client);
			else
				std::cout << "Unknown command: " << cmd << std::endl;
		}
	}
	else
	{
		std::string err = "451 :You have not registered\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
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

void Commands::handleJoin(const std::string& channelName, Client& client)
{
	std::string msg;
	bool	isop = false;
	if (channels.find(channelName) == channels.end())
	{
		channels[channelName] = Channel(channelName);
		std::cout << "New channel created: " << channelName << std::endl;
		channels[channelName].setPwd("");
		channels[channelName].setName(channelName);
		channels[channelName].setInvOnly(false);
		channels[channelName].setMaxUsers(-1);
		channels[channelName].addOp(client.getNickname(), client);
		isop = true;
	}
	else
	{
		if (channels[channelName].getInvOnly())
		{
			if (std::find(channels[channelName].getInvitedUsers().begin(), channels[channelName].getInvitedUsers().end(), client.getNickname()) == channels[channelName].getInvitedUsers().end())
			{
				std::string err = "473 " + client.getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n";
				send(client.getFd(), err.c_str(), err.size(), 0);
				return;
			}
		}
		if (channels[channelName].getMaxUsers() != -1 && channels[channelName].getUsers().size() >= channels[channelName].getMaxUsers())
		{
			std::string err = "471 " + client.getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n";
			send(client.getFd(), err.c_str(), err.size(), 0);
			return;
		}
		std::vector<Client>::iterator it;
		std::vector<Client>& users = channels[channelName].getUsers();
		
		for (it = users.begin(); it != users.end(); ++it)
		{
			if (it->getNickname() == client.getNickname())
			{
				std::string err = "443 " + client.getNickname() + " " + channelName + " :is already on channel\r\n";
				send(client.getFd(), err.c_str(), err.size(), 0);
				return;
			}
		}
	}
	channels[channelName].addUser(client);
	std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " JOIN :" + channelName + "\r\n";
	for (std::vector<Client>::iterator it = channels[channelName].getUsers().begin(); it != channels[channelName].getUsers().end(); ++it)
		send(it->getFd(), joinMsg.c_str(), joinMsg.length(), 0);

	std::string topicMsg = ":server 332 " + client.getNickname() + " " + channelName + channels[channelName].getTopic() + "\r\n";
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
	if (isop)
	{
		std::string modeMsg = ":" + client.getNickname() + " MODE " + channelName + " +o " + client.getNickname() + "\r\n";
		std::vector<Client>& users = channels[channelName].getUsers();
		for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
			send(it->getFd(), modeMsg.c_str(), modeMsg.length(), 0);
	}
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


void Commands::handleUserCommand(const std::string& msg, Client& client)
{

	userInfo info = Parser::userParse(msg);
	if (info.nickName.empty() || info.userName.empty() || info.realName.empty())
	{
		std::string err = "461 " + client.getNickname() + " :Not enough parameters\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	client.setUsername(info.userName);
	client.setRealname(info.realName);
	std::string noticeMsg = "Your username is set to: " + info.userName + "\r\n";
	send(client.getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);
	std::string noticeMsg2 = "Your realname is set to: " + info.realName + "\r\n";
	send(client.getFd(), noticeMsg2.c_str(), noticeMsg2.size(), 0);
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
	std::string welcomeMsg = ":server 001 " + client.getNickname() + " " + client.getNickname() + "\r\n"; 
	send(client.getFd(), welcomeMsg.c_str(), welcomeMsg.size(), 0);
	std::string noticeMsg = "Your nickname is set to: " + nick + "\r\n";
	send(client.getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);
}

void Commands::handleModeCommand(const std::string& msg, Client& client)
{
	modeInfo info = Parser::modeParse(msg);
	if (info.key.empty())
	{
		std::string err = "401 " + client.getNickname() + " :No such channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (isOP(channels[info.channel].getName(), client) == false)
	{
		std::string err = "481 " + client.getNickname() + " :Permission Denied - You're not an operator in this server.\r\n";
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
			std::cout << "Mode change on channel: " << info.channel << " -> " << (info.status ? "+" : "-") << info.key << std::endl;
			if (info.key == "i")
			{
				if (!channels[info.channel].getInvOnly())
					channels[info.channel].setInvOnly(info.status);
				else
					channels[info.channel].setInvOnly(0);
				noticeMsg = client.getNickname() + " has changed the channel " + info.channel + "'s invite only mode." + (info.status ? " Now invite only." : " No longer invite only.");
			}
			else if (info.key == "k")
			{
				channels[info.channel].setPwd(info.parameters);
				noticeMsg = client.getNickname() + " has changed the channel " + info.channel + "'s password to: " + info.parameters;
			}
			else if (info.key == "l")
			{
				int maxUsers = ft_atoi(info.parameters);
				channels[info.channel].setMaxUsers(maxUsers);
				noticeMsg = client.getNickname() + " has changed the channel " + info.channel + "'s max users to: " + ft_itoa(maxUsers);
			}
			else if (info.key == "o")
			{
				channels[info.channel].addOp(info.parameters, client);
				client.setIsop(true);
				noticeMsg = client.getNickname() + " has given operator status to " + info.parameters + " in channel " + info.channel;
			}
			else if (info.key == "t")
			{
				channels[info.channel].setTopic(info.parameters);
				noticeMsg = client.getNickname() + " has changed the topic for channel " + info.channel + " to: " + info.parameters;
			}
			else
			{
				std::string err = "472 " + client.getNickname() + " " + info.key + " :is unknown mode char\r\n";
				send(client.getFd(), err.c_str(), err.size(), 0);
				return;
			}
			for (std::vector<Client>::iterator user = channels[info.channel].getUsers().begin(); user != channels[info.channel].getUsers().end(); ++user)
			{
				std::string formattedNotice = ":server NOTICE " + user->getNickname() + " :" + noticeMsg + "\r\n";
				send(user->getFd(), formattedNotice.c_str(), formattedNotice.size(), 0);
			}
			return;
		}
		++it;
	}
	std::string err = "482 " + client.getNickname() + " " + info.channel + " :You're not channel operator\r\n";
	send(client.getFd(), err.c_str(), err.size(), 0);
}

void Commands::handlePartCommand(const std::string& msg, Client& client)
{
	std::vector<std::string> words = split(msg, " ");
	if (words.size() < 2)
	{
		std::string err = "461 " + client.getNickname() + " :Not enough parameters\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	std::string channelName = words[1];
	if (channels.find(channelName) == channels.end())
	{
		std::string err = "403 " + client.getNickname() + " " + channelName + " :No such channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	std::string noticeMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " PART " + channelName + "\r\n";
	for (std::vector<Client>::iterator it = channels[channelName].getUsers().begin(); it != channels[channelName].getUsers().end(); ++it)
	{
		send(it->getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);
	}
	channels[channelName].removeUser(client);
	if (channels[channelName].getUsers().empty())
		channels.erase(channelName);
}

void Commands::handleQuitCommand(const std::string& msg, Client& client)
{
	std::string quitMsg = ":" + client.getNickname() + " QUIT :" + msg + "\r\n";
	send(client.getFd(), quitMsg.c_str(), quitMsg.size(), 0);
	client.clearBuffer();
	clients.erase(client.getFd());
	std::cout << "Client " << client.getNickname() << " has quit." << std::endl;
}

void Commands::handleTopicCommand(const std::string& msg, Client& client)
{
	std::vector<std::string> words = split(msg, " ");
	if (words.size() < 2)
	{
		std::string err = "461 " + client.getNickname() + " :Not enough parameters\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	std::string channelName = words[1];
	if (channels.find(channelName) == channels.end())
	{
		std::string err = "403 " + client.getNickname() + " " + channelName + " :No such channel\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (words.size() != 2 && !isOP(channelName, client))
	{
		std::string err = "482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n";
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
		std::string topicSetMsg = "The topic for " + channelName + " has been set to: " + msg.substr(12) + "\r\n";
		send(client.getFd(), topicSetMsg.c_str(), topicSetMsg.size(), 0);
		channels[channelName].setTopic(msg.substr(12));
	}
	std::string topicSetMsg = ":server 333 " + client.getNickname() + " " + channelName + " :" + msg.substr(12) + "\r\n";
	send(client.getFd(), topicSetMsg.c_str(), topicSetMsg.size(), 0);
}

void Commands::handleKickCommand(const std::string& msg, Client& client)
{
	std::vector<std::string> words = split(msg, " ");
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
		std::string err = ":" + client.getNickname() + " NOTICE " + client.getNickname() + " :Can't kick yourself lil bro 😭\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (!isOP(channelName, client))
	{
		std::string err = "482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->getNickname() == targetNick)
		{
			// std::string kickMsg = ":" + client.getNickname() + " KICK " + channelName + " " + targetNick + "\r\n";
			// for (std::vector<Client>::iterator user = users.begin(); user != users.end(); ++user)
			// 	send(user->getFd(), kickMsg.c_str(), kickMsg.size(), 0);
			std::string noticeMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " KICK " + channelName + " " + targetNick + "\r\n";
			for (std::vector<Client>::iterator it = channels[channelName].getUsers().begin(); it != channels[channelName].getUsers().end(); ++it)
				send(it->getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);
			channels[channelName].removeUser(*it);
			return;
		}
	}
	std::string err = "441 " + client.getNickname() + " " + targetNick + " :They aren't on that channel\r\n";
	send(client.getFd(), err.c_str(), err.size(), 0);
}


void Commands::handleInviteCommand(const std::string& msg, Client& client)
{
	std::vector<std::string> words = split(msg, " ");
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
