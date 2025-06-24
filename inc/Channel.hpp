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
			std::string				topic;
			std::vector<Client>		users;
			std::vector<std::string>	ops;
			std::vector<std::string>	blacklists;
			bool						invOnly;
			int							maxUsers;
			

	public:
			Channel();
			Channel(const std::string& name);
			~Channel();
			void setName(const std::string& name);
			void setPwd(const std::string& pwd);
			void setInvOnly(const bool& invOnly);
			void setMaxUsers(const int& maxUsers);
			void setTopic(const std::string& topic);

			std::string	getName() const;
			std::string	getPwd() const;
			bool		getInvOnly() const;
			int			getMaxUsers() const;
			std::vector<std::string> getOps();
			std::string getTopic() const;
			void addUser(const Client& user);
			void addOp(const std::string& op);
			void removeUser(const Client& user);
			std::vector<Client>& getUsers();

};

#endif
