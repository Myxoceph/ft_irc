#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include "Server.hpp"
#include "Channel.hpp"


class Client
{
	private:
			int			fd;
			std::string	nickname;
			std::string	username;
			std::string	hostname;
			std::string	realname;
			std::string	servername;
			std::string	buffer;
			bool 		isauth;
			bool		isop;
	public:
			Client(const int& fd);
			~Client();
			int			getFd() const;
			std::string	getNickname() const;
			std::string	getUsername() const;
			std::string	getHostname() const;
			std::string	getRealname() const;
			std::string	getServername() const;
			bool		getIsauth() const;
			bool		getIsop() const;
			bool		hasFullMessage(std::string& out);
			std::string	&getBuffer();

			void appendToBuffer(const std::string& buffer);
			void clearBuffer();

			void		setNickname(const std::string& nickname);
			void		setUsername(const std::string& username);
			void		setHostname(const std::string& hostname);
			void		setRealname(const std::string& realname);
			void		setServername(const std::string& servername);
			void		setIsauth(const bool& isauth);
			void		setIsop(const bool& isop);
};


#endif
