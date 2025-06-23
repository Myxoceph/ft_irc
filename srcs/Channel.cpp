#include "Channel.hpp"

Channel::Channel()
{
	this->name = "";
	this->pwd = "";
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
