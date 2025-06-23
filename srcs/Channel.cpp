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
