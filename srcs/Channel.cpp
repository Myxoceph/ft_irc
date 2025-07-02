#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel()
{
	this->name = "";
	this->pwd = "";
	this->invOnly = false;
	this->topicRestricted = true; // Default to restricted
	this->maxUsers = -1;
	this->topic = "The topic has not been set yet.";
}

Channel::Channel(const std::string& name)
{
	this->name = name;
	this->pwd = "";
	this->invOnly = false;
	this->topicRestricted = true; // Default to restricted
	this->maxUsers = -1;
	this->topic = "The topic has not been set yet.";
}

Channel::~Channel()
{

}

void Channel::setName(const std::string& name)
{
	this->name = name;
}

void Channel::setPwd(const std::string& pwd)
{
	this->pwd = pwd;
}

void Channel::setInvOnly(const bool& invOnly)
{
	this->invOnly = invOnly;
}

void Channel::setTopicRestricted(const bool& topicRestricted)
{
	this->topicRestricted = topicRestricted;
}

void Channel::setMaxUsers(const int& maxUsers)
{
	this->maxUsers = maxUsers;
}

bool Channel::getInvOnly() const
{
	return (this->invOnly);
}

bool Channel::getTopicRestricted() const
{
	return (this->topicRestricted);
}

int Channel::getMaxUsers() const
{
	return (this->maxUsers);
}

void Channel::addOp(Client* user)
{
	if (!isOp(user))
		ops.push_back(user);
}

void Channel::removeOp(Client* user)
{
	std::vector<Client*>::iterator it = std::find(ops.begin(), ops.end(), user);
	if (it != ops.end())
		ops.erase(it);
}

std::string Channel::getName() const
{
	return (this->name);
}

std::string Channel::getPwd() const
{
	return (this->pwd);
}

void Channel::addUser(Client* user)
{
	for (std::vector<Client*>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if ((*it)->getNickname() == user->getNickname())
		{
			return;
		}
	}
	users.push_back(user);
}


void Channel::removeUser(Client* user)
{
	// Remove from users list
	std::vector<Client*>::iterator it = std::find(users.begin(), users.end(), user);
	if (it != users.end())
	{
		users.erase(it);
	}
	
	// Also remove from operators list if they were an operator
	removeOp(user);
}

std::vector<Client*>& Channel::getUsers()
{
	return (this->users);
}

std::vector<Client*>& Channel::getOps()
{
	return (this->ops);
}

std::string Channel::getTopic() const
{
	return (this->topic);
}

void Channel::setTopic(const std::string& topic)
{
	this->topic = topic;
}

std::vector<std::string> Channel::getInvitedUsers() const
{
	return (this->invitedUsers);
}

void Channel::addInvitedUser(const std::string& user)
{
	if (std::find(invitedUsers.begin(), invitedUsers.end(), user) == invitedUsers.end())
	{
		invitedUsers.push_back(user);
	}
}

bool Channel::isOp(Client* user) const
{
	return (std::find(ops.begin(), ops.end(), user) != ops.end());
}

bool Channel::isUserInChannel(const std::string& nickname) const
{
	for (std::vector<Client*>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		if ((*it)->getNickname() == nickname)
			return true;
	}
	return false;
}

bool Channel::isInvited(const std::string& nickname) const
{
	return (std::find(invitedUsers.begin(), invitedUsers.end(), nickname) != invitedUsers.end());
}

void Channel::broadcast(const std::string& message)
{
	for (std::vector<Client*>::iterator it = users.begin(); it != users.end(); ++it)
	{
		send((*it)->getFd(), message.c_str(), message.size(), 0);
	}
}

void Channel::broadcast(const std::string& message, Client* exclude)
{
	for (std::vector<Client*>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (*it != exclude)
			send((*it)->getFd(), message.c_str(), message.size(), 0);
	}
}
