#include "Channel.hpp"
#include "Client.hpp"
#include <sys/socket.h>

Channel::Channel()
{
	this->name = "";
	this->pwd = "";
	this->invOnly = false;
	this->maxUsers = -1;
	this->topic = "The topic has not been set yet.";
	this->topicSet = false;
}

Channel::Channel(const std::string& name)
{
	this->name = name;
	this->pwd = "";
	this->invOnly = false;
	this->maxUsers = -1;
	this->topic = "The topic has not been set yet.";
	this->topicSet = false;
}

Channel::~Channel()
{
	users.clear();
	ops.clear();
	invitedUsers.clear();
	this->name.clear();
	this->pwd.clear();
	this->topic.clear();
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

void Channel::setMaxUsers(const int& maxUsers)
{
	this->maxUsers = maxUsers;
}

bool Channel::getInvOnly() const
{
	return (this->invOnly);
}

int Channel::getMaxUsers() const
{
	return (this->maxUsers);
}

void Channel::addOp(const std::string& op)
{
	ops.push_back(op);
}

std::string Channel::getName() const
{
	return (this->name);
}

std::string Channel::getPwd() const
{
	return (this->pwd);
}

void Channel::addUser(const Client& user)
{
	for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
		if (it->getNickname() == user.getNickname())
			return;
	users.push_back(user);
}


void Channel::removeUser(const Client& user)
{
	std::vector<Client>::iterator it = users.begin();
	std::vector<Client>::iterator ite = users.end();
	while (it != ite)
	{
		if (it->getNickname() == user.getNickname())
		{
			users.erase(it);
			return;
		}
		++it;
	}
}

std::vector<Client>& Channel::getUsers()
{
	return (this->users);
}

std::vector<std::string> &Channel::getOps()
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

void Channel::addinvitedUser(const std::string& user)
{
	if (std::find(invitedUsers.begin(), invitedUsers.end(), user) == invitedUsers.end())
		invitedUsers.push_back(user);
}

bool Channel::isOp(const std::string &nickName) const
{
	return (std::find(ops.begin(), ops.end(), nickName) != ops.end());
}

void Channel::removeOp(const std::string& op)
{
	std::vector<std::string>::iterator it = std::find(ops.begin(), ops.end(), op);
	if (it == ops.end())
		return;

	ops.erase(it);
	if (!ops.empty())
		return;

	std::vector<Client> newlist = users;
	for (std::vector<Client>::iterator iter = newlist.begin(); iter != newlist.end(); ++iter)
	{
		if (iter->getNickname() == op)
		{
			newlist.erase(iter);
			break;
		}
	}

	if (newlist.empty())
		return;

	Client& newOpClient = newlist[rand() % newlist.size()];
	std::string newOp = newOpClient.getNickname();
	ops.push_back(newOp);

	std::string ModeMsg = ":Server MODE " + this->name + " +o " + newOp + "\r\n";
	for (std::vector<Client>::iterator user = users.begin(); user != users.end(); ++user)
		send(user->getFd(), ModeMsg.c_str(), ModeMsg.size(), 0);
}

bool Channel::getTopicSet() const
{
	return this->topicSet;
}

void Channel::setTopicSet(const bool& topicSet)
{
	this->topicSet = topicSet;
}

bool Channel::isUserInChannel(const std::string &check) const
{
	std::vector<Client>::const_iterator it = users.begin();
	std::vector<Client>::const_iterator ite = users.end();
	while (it != ite)
	{
		if (it->getNickname() == check)
			return true;
		++it;
	}
	return false;
}
