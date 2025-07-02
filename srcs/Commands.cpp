#include "Commands.hpp"
#include "Parser.hpp"
#include <iostream>

void Commands::sendReply(const Client& client, const std::string& reply)
{
	send(client.getFd(), reply.c_str(), reply.size(), 0);
}

Commands::Commands(std::map<int, Client>& c, std::map<std::string, Channel>& ch)
	: _clients(c), _channels(ch) 
{
	initializeCommands();
}

void Commands::initializeCommands()
{
	_commandHandlers["PASS"] = &Commands::handlePass;
	_commandHandlers["NICK"] = &Commands::handleNick;
	_commandHandlers["USER"] = &Commands::handleUser;
	_commandHandlers["JOIN"] = &Commands::handleJoin;
	_commandHandlers["PRIVMSG"] = &Commands::handlePrivmsg;
	_commandHandlers["MODE"] = &Commands::handleMode;
	_commandHandlers["PART"] = &Commands::handlePart;
	_commandHandlers["QUIT"] = &Commands::handleQuit;
	_commandHandlers["TOPIC"] = &Commands::handleTopic;
	_commandHandlers["KICK"] = &Commands::handleKick;
	_commandHandlers["INVITE"] = &Commands::handleInvite;
}

bool Commands::executeCommand(const std::string& raw, Client& client)
{
	parseInfo info = Parser::parse(raw);
	std::string cmd = info.command;

	if (cmd != "PASS" && !client.getIsAuth())
	{
		sendReply(client, "451 :You have not registered\r\n");
		return true;
	}

	if (cmd != "PASS" && cmd != "NICK" && cmd != "USER" && !client.isProvided())
	{
		sendReply(client, "Please provide your nickname and/or username by using /NICK <nickname> and/or /USER <'username' 0 * :'realname'>\r\n");
		return true;
	}

	std::map<std::string, CommandHandler>::iterator it = _commandHandlers.find(cmd);
	if (it != _commandHandlers.end())
	{
		(this->*(it->second))(info, client);
		if (info.command == "QUIT")
			return false;
	}
	else
	{
		std::cout << "Unknown command: " << cmd << std::endl;
	}
	return true;
}

void Commands::broadcast(const std::string& channelName, const std::string& message, bool excludeSender, const Client& sender)
{
	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	if (it != _channels.end())
	{
		std::vector<Client*>& users = it->second.getUsers();
		for (std::vector<Client*>::iterator user_it = users.begin(); user_it != users.end(); ++user_it)
		{
			if (excludeSender && (*user_it)->getFd() == sender.getFd())
				continue;
			sendReply(**user_it, message);
		}
	}
}

void Commands::handlePass(const parseInfo& info, Client& client)
{
	if (client.getIsAuth())
	{
		sendReply(client, "462 " + client.getNickname() + " :You may not reregister\r\n");
		return;
	}
	if (info.params.empty())
	{
		sendReply(client, "461 " + client.getNickname() + " PASS :Not enough parameters\r\n");
		return;
	}
	if (info.params[0] != client.getPwd())
	{
		sendReply(client, "464 :Password incorrect\r\n");
		return;
	}
	client.setIsAuth(true);
	sendReply(client, "Welcome to the Concord. You are now registered.\r\n");
}

void Commands::handleNick(const parseInfo& info, Client& client)
{
	if (info.params.empty())
	{
		sendReply(client, "431 :No nickname given\r\n");
		return;
	}
	const std::string& nick = info.params[0];
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == nick)
		{
			sendReply(client, "433 " + nick + " :Nickname is already in use\r\n");
			return;
		}
	}
	client.setNickname(nick);
	std::cout << "Nickname set to: " << nick << std::endl;
	if (client.isProvided())
	{
		std::string welcomeMsg = ":server 001 " + client.getNickname() + " :Welcome to the IRC server, " + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + "\r\n";
		sendReply(client, welcomeMsg);
	}
}

void Commands::handleUser(const parseInfo& info, Client& client)
{
	if (info.params.size() < 4)
	{
		sendReply(client, "461 " + client.getNickname() + " USER :Not enough parameters\r\n");
		return;
	}
	client.setUsername(info.params[0]);
	client.setRealname(info.params[3]);
	std::cout << "Username set to: " << info.params[0] << " and realname to: " << info.params[3] << std::endl;
	if (client.isProvided())
	{
		std::string welcomeMsg = ":server 001 " + client.getNickname() + " :Welcome to the IRC server, " + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + "\r\n";
		sendReply(client, welcomeMsg);
	}
}

void Commands::handleJoin(const parseInfo& info, Client& client)
{
	if (info.params.empty())
	{
		sendReply(client, "461 " + client.getNickname() + " JOIN :Not enough parameters\r\n");
		return;
	}
	const std::string& channelName = info.params[0];
	const std::string& key = info.params.size() > 1 ? info.params[1] : "";

	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	bool isop = false;

	if (it == _channels.end())
	{
		_channels[channelName] = Channel(channelName);
		_channels[channelName].addOp(&client);
		isop = true;
		std::cout << "New channel created: " << channelName << std::endl;
	}
	else
	{
		Channel& channel = it->second;

		if (!channel.getPwd().empty() && channel.getPwd() != key)
		{
			sendReply(client, "475 " + client.getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n");
			return;
		}

		if (channel.getInvOnly() && !channel.isInvited(client.getNickname()))
		{
			sendReply(client, "473 " + client.getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n");
			return;
		}
		if (channel.getMaxUsers() != -1 && static_cast<int>(channel.getUsers().size()) >= channel.getMaxUsers())
		{
			sendReply(client, "471 " + client.getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n");
			return;
		}
		if (channel.isUserInChannel(client.getNickname()))
		{
			sendReply(client, "443 " + client.getNickname() + " " + channelName + " :is already on channel\r\n");
			return;
		}
	}

	Channel& channel = _channels[channelName];
	channel.addUser(&client);
	std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " JOIN :" + channelName + "\r\n";
	broadcast(channelName, joinMsg, false, client);

	sendReply(client, ":server 332 " + client.getNickname() + " " + channelName + " :" + channel.getTopic() + "\r\n");

	std::string names;
	const std::vector<Client*>& users = channel.getUsers();
	for (std::vector<Client*>::const_iterator it_user = users.begin(); it_user != users.end(); ++it_user)
	{
		if (channel.isOp(*it_user))
			names += "@" + (*it_user)->getNickname() + " ";
		else
			names += (*it_user)->getNickname() + " ";
	}
	sendReply(client, ":server 353 " + client.getNickname() + " = " + channelName + " :" + names + "\r\n");
	sendReply(client, ":server 366 " + client.getNickname() + " " + channelName + " :End of /NAMES list.\r\n");

	if (isop)
	{
		std::string modeMsg = ":" + client.getNickname() + " MODE " + channelName + " +o " + client.getNickname() + "\r\n";
		broadcast(channelName, modeMsg, false, client);
	}
}

void Commands::handlePrivmsg(const parseInfo& info, Client& sender)
{
	if (info.params.size() < 2)
	{
		sendReply(sender, "411 " + sender.getNickname() + " :No recipient given or no text to send\r\n");
		return;
	}
	const std::string& target = info.params[0];
	const std::string& message = info.params[1];
	std::string fullMessage = ":" + sender.getNickname() + "!" + sender.getUsername() + "@" + sender.getHostname() + " PRIVMSG " + target + " :" + message + "\r\n";

	if (target[0] == '#')
	{
		broadcast(target, fullMessage, true, sender);
	}
	else
	{
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			if (it->second.getNickname() == target)
			{
				sendReply(it->second, fullMessage);
				return;
			}
		}
		sendReply(sender, "401 " + sender.getNickname() + " " + target + " :No such nick/channel\r\n");
	}
}

void Commands::handleMode(const parseInfo& info, Client& client)
{
	if (info.params.size() < 1)
	{
		sendReply(client, "461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
		return;
	}

	const std::string& target = info.params[0];

	// We only handle channel modes for now
	if (target[0] != '#')
	{
		// Handle user modes if necessary, for now, we ignore them.
		return;
	}

	std::map<std::string, Channel>::iterator it = _channels.find(target);
	if (it == _channels.end())
	{
		sendReply(client, "403 " + client.getNickname() + " " + target + " :No such channel\r\n");
		return;
	}

	Channel& channel = it->second;

	// If no mode string is provided, reply with the current channel modes.
	if (info.params.size() == 1)
	{
		std::string modes = "+k"; // Assuming key is always set-able, adjust if needed
		if (channel.getInvOnly()) modes += "i";
		if (channel.getTopicRestricted()) modes += "t";
		if (channel.getMaxUsers() != -1) modes += "l";
		sendReply(client, "324 " + client.getNickname() + " " + target + " " + modes + "\r\n");
		return;
	}

	if (!channel.isOp(&client))
	{
		sendReply(client, "482 " + client.getNickname() + " " + target + " :You're not channel operator\r\n");
		return;
	}

	const std::string& modeStr = info.params[1];
	bool adding = true;
	size_t paramIndex = 2;

	for (std::string::const_iterator mode_it = modeStr.begin(); mode_it != modeStr.end(); ++mode_it)
	{
		char mode = *mode_it;
		if (mode == '+')
		{
			adding = true;
			continue;
		}
		else if (mode == '-')
		{
			adding = false;
			continue;
		}

		std::string param;
		if (paramIndex < info.params.size())
		{
			param = info.params[paramIndex];
		}

		switch (mode)
		{
			case 'i':
				channel.setInvOnly(adding);
				broadcast(target, ":" + client.getNickname() + " MODE " + target + (adding ? " +i" : " -i") + "\r\n", false, client);
				break;
			case 't':
				channel.setTopicRestricted(adding);
				broadcast(target, ":" + client.getNickname() + " MODE " + target + (adding ? " +t" : " -t") + "\r\n", false, client);
				break;
			case 'k':
				if (adding)
				{
					if (param.empty())
					{
						sendReply(client, "461 " + client.getNickname() + " MODE :Not enough parameters for +k\r\n");
						continue;
					}
					channel.setPwd(param);
					broadcast(target, ":" + client.getNickname() + " MODE " + target + " +k " + param + "\r\n", false, client);
					paramIndex++;
				}
				else
				{
					channel.setPwd("");
					broadcast(target, ":" + client.getNickname() + " MODE " + target + " -k\r\n", false, client);
				}
				break;
			case 'o':
			{
				if (param.empty())
				{
					sendReply(client, "461 " + client.getNickname() + " MODE :Not enough parameters for +/-o\r\n");
					continue;
				}
				Client* targetUser = NULL;
				for (std::map<int, Client>::iterator c_it = _clients.begin(); c_it != _clients.end(); ++c_it)
				{
					if (c_it->second.getNickname() == param)
					{
						targetUser = &c_it->second;
						break;
					}
				}
				if (!targetUser || !channel.isUserInChannel(targetUser->getNickname()))
				{
					sendReply(client, "441 " + client.getNickname() + " " + param + " " + target + " :They aren't on that channel\r\n");
					continue;
				}
				if (adding)
					channel.addOp(targetUser);
				else
					channel.removeOp(targetUser);
				broadcast(target, ":" + client.getNickname() + " MODE " + target + (adding ? " +o " : " -o ") + param + "\r\n", false, client);
				paramIndex++;
				break;
			}
			case 'l':
				if (adding)
				{
					if (param.empty())
					{
						sendReply(client, "461 " + client.getNickname() + " MODE :Not enough parameters for +l\r\n");
						continue;
					}
					char* end;
					long limit = std::strtol(param.c_str(), &end, 10);
					if (*end != '\0' || limit <= 0)
					{
						sendReply(client, "472 " + client.getNickname() + " " + target + " :is not a valid limit for a channel\r\n");
						continue;
					}
					channel.setMaxUsers(limit);
					broadcast(target, ":" + client.getNickname() + " MODE " + target + " +l " + param + "\r\n", false, client);
					paramIndex++;
				}
				else
				{
					channel.setMaxUsers(-1);
					broadcast(target, ":" + client.getNickname() + " MODE " + target + " -l\r\n", false, client);
				}
				break;
			default:
				sendReply(client, "482 " + client.getNickname() + " " + target + " :is an unknown mode char to me\r\n");
				break;
		}
	}
}

void Commands::handlePart(const parseInfo& info, Client& client)
{
	if (info.params.empty())
	{
		sendReply(client, "461 " + client.getNickname() + " PART :Not enough parameters\r\n");
		return;
	}

	const std::string& channelName = info.params[0];
	std::string reason = info.params.size() > 1 ? info.params[1] : "Leaving";

	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	if (it == _channels.end())
	{
		sendReply(client, "403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	Channel& channel = it->second;
	if (!channel.isUserInChannel(client.getNickname()))
	{
		sendReply(client, "442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	std::string partMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " PART " + channelName + " :" + reason + "\r\n";
	broadcast(channelName, partMsg, false, client);

	channel.removeUser(&client);

	if (channel.getUsers().empty())
	{
		_channels.erase(it);
		std::cout << "Channel " << channelName << " is empty and has been removed." << std::endl;
	}
}

void Commands::handleQuit(const parseInfo& info, Client& client)
{
	std::string quitMessage = info.params.empty() ? "Client quit" : info.params[0];
	std::string quitBroadcast = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " QUIT :" + quitMessage + "\r\n";

	// Use a temporary copy of channel names to avoid iterator invalidation issues
	std::vector<std::string> channels_to_part;
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->second.isUserInChannel(client.getNickname()))
		{
			channels_to_part.push_back(it->first);
		}
	}

	for (std::vector<std::string>::iterator it = channels_to_part.begin(); it != channels_to_part.end(); ++it)
	{
		broadcast(*it, quitBroadcast, true, client);
		Channel& channel = _channels.at(*it);
		channel.removeUser(&client);
		if (channel.getUsers().empty())
		{
			_channels.erase(*it);
			std::cout << "Channel " << *it << " is empty and has been removed." << std::endl;
		}
	}

	std::string errorMsg = "ERROR :Closing link: (" + client.getUsername() + "@" + client.getHostname() + ") [" + quitMessage + "]\r\n";
	sendReply(client, errorMsg);
}

void Commands::handleTopic(const parseInfo& info, Client& client)
{
	if (info.params.empty())
	{
		sendReply(client, "461 " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
		return;
	}

	const std::string& channelName = info.params[0];
	std::map<std::string, Channel>::iterator it = _channels.find(channelName);

	if (it == _channels.end())
	{
		sendReply(client, "403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	Channel& channel = it->second;
	if (!channel.isUserInChannel(client.getNickname()))
	{
		sendReply(client, "442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	if (info.params.size() > 1)
	{
		// Setting the topic
		if (channel.getTopicRestricted() && !channel.isOp(&client))
		{
			sendReply(client, "482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
			return;
		}

		if (channel.isOp(&client)) // Assuming a mode to restrict topic changes is not implemented yet
		{
			const std::string& newTopic = info.params[1];
			channel.setTopic(newTopic);
			std::string topicMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
			broadcast(channelName, topicMsg, false, client);
		}
		else
		{
			sendReply(client, "482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
		}
	}
	else
	{
		// Getting the topic
		const std::string& topic = channel.getTopic();
		if (topic.empty())
		{
			sendReply(client, "331 " + client.getNickname() + " " + channelName + " :No topic is set\r\n");
		}
		else
		{
			sendReply(client, "332 " + client.getNickname() + " " + channelName + " :" + topic + "\r\n");
		}
	}
}

void Commands::handleKick(const parseInfo& info, Client& client)
{
	if (info.params.size() < 2)
	{
		sendReply(client, "461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
		return;
	}

	const std::string& channelName = info.params[0];
	const std::string& targetNick = info.params[1];
	std::string reason = info.params.size() > 2 ? info.params[2] : "Kicked by operator";

	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	if (it == _channels.end())
	{
		sendReply(client, "403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	Channel& channel = it->second;

	if (!channel.isUserInChannel(client.getNickname()))
	{
		sendReply(client, "442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	if (!channel.isOp(&client))
	{
		sendReply(client, "482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	Client* targetClient = NULL;
	std::vector<Client*>& users = channel.getUsers();
	for (std::vector<Client*>::iterator user_it = users.begin(); user_it != users.end(); ++user_it)
	{
		if ((*user_it)->getNickname() == targetNick)
		{
			targetClient = *user_it;
			break;
		}
	}

	if (targetClient == NULL)
	{
		sendReply(client, "441 " + client.getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
		return;
	}

	std::string kickMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
	broadcast(channelName, kickMsg, false, client);

	channel.removeUser(targetClient);

	if (channel.getUsers().empty())
	{
		_channels.erase(it);
		std::cout << "Channel " << channelName << " is empty and has been removed." << std::endl;
	}
}

void Commands::handleInvite(const parseInfo& info, Client& client)
{
	if (info.params.size() < 2)
	{
		sendReply(client, "461 " + client.getNickname() + " INVITE :Not enough parameters\r\n");
		return;
	}

	const std::string& targetNick = info.params[0];
	const std::string& channelName = info.params[1];

	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	if (it == _channels.end())
	{
		sendReply(client, "403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	Channel& channel = it->second;

	if (!channel.isUserInChannel(client.getNickname()))
	{
		sendReply(client, "442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	if (channel.getInvOnly() && !channel.isOp(&client))
	{
		sendReply(client, "482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	Client* targetClient = NULL;
	for (std::map<int, Client>::iterator client_it = _clients.begin(); client_it != _clients.end(); ++client_it)
	{
		if (client_it->second.getNickname() == targetNick)
		{
			targetClient = &client_it->second;
			break;
		}
	}

	if (targetClient == NULL)
	{
		sendReply(client, "401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
		return;
	}

	if (channel.isUserInChannel(targetNick))
	{
		sendReply(client, "443 " + client.getNickname() + " " + targetNick + " " + channelName + " :is already on channel\r\n");
		return;
	}

	channel.addInvitedUser(targetNick);

	// RPL_INVITING
	sendReply(client, ":server 341 " + client.getNickname() + " " + targetNick + " " + channelName + "\r\n");

	// INVITE message to target user
	std::string inviteMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() + " INVITE " + targetNick + " :" + channelName + "\r\n";
	sendReply(*targetClient, inviteMsg);

	std::cout << "User " << client.getNickname() << " invited " << targetNick << " to channel " << channelName << std::endl;
}

// NOTE: The old command handlers below will be removed in the next step.
