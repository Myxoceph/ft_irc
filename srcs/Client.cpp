#include "Client.hpp"


Client::Client(const int& fd)
{
	this->fd = fd;
	this->isauth = false;
	this->isop = false;
}

Client::~Client()
{

}

void Client::setNickname(const std::string& nickname)
{
	this->nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	this->username = username;
}

void Client::setHostname(const std::string& hostname)
{
	this->hostname = hostname;
}

void Client::setRealname(const std::string& realname)
{
	this->realname = realname;
}

void Client::setServername(const std::string& servername)
{
	this->servername = servername;
}

void Client::setIsauth(const bool& isauth)
{
	this->isauth = isauth;
}

void Client::setIsop(const bool& isop)
{
	this->isop = isop;
}

int Client::getFd() const
{
	return (this->fd);
}

std::string Client::getNickname() const
{
	return (this->nickname);
}

std::string Client::getUsername() const
{
	return (this->username);
}

std::string Client::getHostname() const
{
	return (this->hostname);
}

std::string Client::getRealname() const
{
	return (this->realname);
}

std::string Client::getServername() const
{
	return (this->servername);
}

bool Client::getIsauth() const
{
	return (this->isauth);
}

bool Client::getIsop() const
{
	return (this->isop);
}
