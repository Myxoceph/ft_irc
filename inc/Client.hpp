#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <algorithm>
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
			bool		sentNick;
			bool		hasUsername;
			bool		isop;

			std::vector<std::string> joined_channels;
	public:
			Client(const int& fd);
			~Client();
			int			getFd() const;
			std::string	getNickname() const;
			std::string	getUsername() const;
			std::string	getHostname() const;
			std::string	getRealname() const;
			std::string	getServername() const;
			bool		getIsop() const;
			bool		hasFullMessage(std::string& out);
			bool		isAuthenticated() const;
			std::string	&getBuffer();

			void 		appendToBuffer(const std::string& buffer);
			void 		clearBuffer();

			void		setNickname(const std::string& nickname);
			void		setUsername(const std::string& username);
			void		setHostname(const std::string& hostname);
			void		setRealname(const std::string& realname);
			void		setServername(const std::string& servername);
			void		setIsop(const bool& isop);

			void		joinChannel(const std::string& channel);
			void		partChannel(const std::string& channel);
			const std::vector<std::string>& getJoinedChannels() const;
};


#endif
