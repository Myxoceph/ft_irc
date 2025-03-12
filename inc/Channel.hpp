#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <set>
#include "Server.hpp"
#include "Client.hpp"

class Channel
{
	private:
			std::string				name;
			std::string				pwd;
			std::set<std::string>	users;
			std::set<std::string>	ops;
			std::set<std::string>	blacklists;
	public:
			Channel(const std::string& name);
			~Channel();
			void setName(const std::string& name);
			void setPwd(const std::string& pwd);

			std::string	getName() const;
			std::string	getPwd() const;

};

#endif
