#include "Client.hpp"


Client::Client(const int& fd)
{
	this->fd = fd;
	this->sentNick = false;
	this->hasUsername = false;
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

bool Client::isAuthenticated() const
{
	return (this->sentNick && this->hasUsername);
}

const std::vector<std::string>& Client::getJoinedChannels() const
{
	return (this->joined_channels);
}

void Client::joinChannel(const std::string& channel)
{
	if (std::find(joined_channels.begin(), joined_channels.end(), channel) == joined_channels.end())
		joined_channels.push_back(channel);
}

void Client::partChannel(const std::string& channel)
{
	std::vector<std::string>::iterator it = std::find(joined_channels.begin(), joined_channels.end(), channel);
	if (it != joined_channels.end())
		joined_channels.erase(it);
}

bool Client::getIsop() const
{
	return (this->isop);
}

std::string& Client::getBuffer()
{
	return buffer;
}

void Client::appendToBuffer(const std::string& data)
{
	buffer += data;
}

void Client::clearBuffer()
{
	buffer.clear();
}

bool Client::hasFullMessage(std::string& out)
{
	size_t pos = buffer.find("\r\n");
	if (pos == std::string::npos)
		return false;

	out = buffer.substr(0, pos + 2);
	buffer.erase(0, pos + 2);
	return true;
}
