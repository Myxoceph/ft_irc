#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

class Client
{
	private:
		// Client attributes
		int			fd;
			std::string	nickname;
			std::string	username;
			std::string	hostname;
			std::string	realname;
			std::string pwd;
			bool		isAuth;
			bool		isop;

			// Buffer for incoming data
			std::string	buffer;

	public:
			// Constructors and Destructors
			Client(const int& fd);
			~Client();

			// Getters
			int			getFd() const;
			std::string	getNickname() const;
			std::string	getUsername() const;
			std::string	getHostname() const;
			std::string	getRealname() const;
			std::string	getPwd() const;
			bool		getIsop() const;
			bool		getIsAuth() const;
			std::string	&getBuffer();

			// Setters
			void		setNickname(const std::string& nickname);
			void		setUsername(const std::string& username);
			void		setHostname(const std::string& hostname);
			void		setRealname(const std::string& realname);
			void		setIsop(const bool& isop);
			void		setIsAuth(const bool& isAuth);
			void		setPwd(const std::string& pwd);

			// Buffer management
			void 		appendToBuffer(const std::string& data);
			void 		clearBuffer();
			bool		hasFullMessage(std::string& out);

			// Other methods
			bool		isProvided() const;
};


#endif
