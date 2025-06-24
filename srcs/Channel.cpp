#include "Channel.hpp"

Channel::Channel()
{
	this->name = "";
	this->pwd = "";
	this->invOnly = false;
	this->maxUsers = -1;
}

Channel::Channel(const std::string& name)
{
	this->name = name;
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
	std::string modeMsg = ":server_name MODE #test +o alice\r\n";
	for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
		send(it->getFd(), modeMsg.c_str(), modeMsg.size(), 0);
	std::cout << op << " has been given operator in channel " << this->name << std::endl;
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
	{
		if (it->getNickname() == user.getNickname())
		{
			std::cout << "User " << user.getNickname() << " is already in the channel " << this->name << std::endl;
			return;
		}
	}
	users.push_back(user);
	std::cout << "User " << user.getNickname() << " added to channel " << this->name << std::endl;
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
			std::cout << "User " << user.getNickname() << " removed from channel " << this->name << std::endl;
			return;
		}
		++it;
	}
	std::cout << "User " << user.getNickname() << " not found in channel " << this->name << std::endl;
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
	std::cout << "Topic for channel " << this->name << " set to: " << topic << std::endl;
}

std::vector<std::string> Channel::getInvitedUsers() const
{
	return (this->invitedUsers);
}

void Channel::addinvitedUser(const std::string& user)
{
	if (std::find(invitedUsers.begin(), invitedUsers.end(), user) == invitedUsers.end())
	{
		invitedUsers.push_back(user);
		std::cout << "User " << user << " added to the invited list of channel " << this->name << std::endl;
	}
	else
	{
		std::cout << "User " << user << " is already in the invited list of channel " << this->name << std::endl;
	}
}
