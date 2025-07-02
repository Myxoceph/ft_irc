#include "Client.hpp"


// Constructors and Destructors
Client::Client(const int& fd)
{
	this->fd = fd;
	this->isop = false;
	this->isAuth = false;
	this->hostname = "localhost"; // Initialize hostname
}

Client::~Client()
{

}


// Getters
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

std::string Client::getPwd() const
{
	return this->pwd;
}

bool Client::getIsop() const
{
	return (this->isop);
}

bool Client::getIsAuth() const
{
	return this->isAuth;
}

std::string& Client::getBuffer()
{
	return buffer;
}


// Setters
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

void Client::setIsop(const bool& isop)
{
	this->isop = isop;
}

void Client::setIsAuth(const bool& isAuth)
{
	this->isAuth = isAuth;
}

void Client::setPwd(const std::string& pwd)
{
	this->pwd = pwd;
}


// Buffer management
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


// Other methods
bool Client::isProvided() const
{
	return !this->nickname.empty() && !this->username.empty() && !this->realname.empty();
}
