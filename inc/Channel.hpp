#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <set>
#include <vector>
#include "Server.hpp"
#include "Client.hpp"

class Client;

class Channel
{
	private:
			std::string				name;
			std::string				pwd;
			std::vector<Client>		users;
			std::vector<std::string>	ops;
			std::vector<std::string>	blacklists;
	public:
			Channel();
			Channel(const std::string& name);
			~Channel();
			void setName(const std::string& name);
			void setPwd(const std::string& pwd);

			std::string	getName() const;
			std::string	getPwd() const;
			void addUser(const Client& user);
			void removeUser(const Client& user);
			std::vector<Client>& getUsers();

};

#endif
