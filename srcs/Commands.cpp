#include "Commands.hpp"
#include "Parser.hpp"
#include <iostream>

static int stringToInt(const std::string& str)
{
	std::stringstream ss(str);
	int num;
	ss >> num;
	return num;
}

Commands::Commands(std::map<int, Client>& c, std::map<std::string, Channel>& ch)
	: clients(c), channels(ch) {}

void Commands::executeCommand(const std::string& raw, Client& client)
{
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
	if (channels.find(channelName) == channels.end())
	{
		channels[channelName] = Channel(channelName);
		std::cout << "New channel created: " << channelName << std::endl;
		channels[channelName].setPwd("");
		channels[channelName].setName(channelName);
		channels[channelName].setInvOnly(false);
		channels[channelName].setMaxUsers(-1);
		channels[channelName].addOp(client.getNickname());
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
	send(client.getFd(), joinMsg.c_str(), joinMsg.length(), 0);

	std::string topicMsg = ":server 332 " + client.getNickname() + " " + channelName + " :No topic is set\r\n";
	send(client.getFd(), topicMsg.c_str(), topicMsg.length(), 0);

	std::string namesMsg = ":server 353 " + client.getNickname() + " = " + channelName + " :" + client.getNickname() + "\r\n";
	send(client.getFd(), namesMsg.c_str(), namesMsg.length(), 0);

	std::string endNames = ":server 366 " + client.getNickname() + " " + channelName + " :End of /NAMES list.\r\n";
	send(client.getFd(), endNames.c_str(), endNames.length(), 0);
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
	if (isOP(channels[info.channel].getName(), client) == false)
	{
		std::string err = "481 " + client.getNickname() + " :Permission Denied - You're not an operator in this server.\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	std::vector<std::string> &ops = channels[info.channel].getOps();
	std::vector<std::string>::iterator it = ops.begin();
	std::vector<std::string>::iterator ite = ops.end();
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
			}
			else if (info.key == "k")
				channels[info.channel].setPwd(info.parameters);
			else if (info.key == "l")
			{
				int maxUsers = stringToInt(info.parameters);
				channels[info.channel].setMaxUsers(maxUsers);
			}
			else if (info.key == "o")
			{
				channels[info.channel].addOp(info.parameters);
				client.setIsop(true);
			}
			else if (info.key == "t")
			{
				channels[info.channel].setTopic(info.parameters);
			}
			else
			{
				std::string err = "472 " + client.getNickname() + " " + info.key + " :is unknown mode char\r\n";
				send(client.getFd(), err.c_str(), err.size(), 0);
				return;
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
	channels[channelName].removeUser(client);
	std::string partMsg = ":" + client.getNickname() + " PART " + channelName + "\r\n";
	send(client.getFd(), partMsg.c_str(), partMsg.size(), 0);
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
	if (!isOP(channelName, client))
	{
		std::string err = "482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n";
		send(client.getFd(), err.c_str(), err.size(), 0);
		return;
	}
	if (words.size() == 2)
	{
		std::string topic = channels[channelName].getTopic();
		if (topic.empty())
			topic = "No topic is set.";
		std::string topicMsg = ":server 332 " + client.getNickname() + " " + channelName + " :" + topic + "\r\n";
		send(client.getFd(), topicMsg.c_str(), topicMsg.size(), 0);
		return;
	}
	channels[channelName].setTopic(words[2]);
	std::string topicSetMsg = ":server 333 " + client.getNickname() + " " + channelName + " :" + words[2] + "\r\n";
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
			std::string kickMsg = ":" + client.getNickname() + " KICK " + channelName + " " + targetNick + "\r\n";
			for (std::vector<Client>::iterator user = users.begin(); user != users.end(); ++user)
				send(user->getFd(), kickMsg.c_str(), kickMsg.size(), 0);
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
