#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

class Client;

class Channel
{
	private:
			std::string				name;
			std::string				pwd;
			std::string				topic;
		std::vector<Client>		users;
		std::vector<std::string>	ops;
		std::vector<std::string>	invitedUsers;
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
			void addinvitedUser(const std::string& user);

			std::string	getName() const;
			std::string	getPwd() const;
			bool		getInvOnly() const;
			bool		isOp(const std::string& nickName) const;
			int			getMaxUsers() const;
			std::vector<std::string> &getOps();
			std::string getTopic() const;
			std::vector<std::string> getInvitedUsers() const;
			void addUser(const Client& user);
			void addOp(const std::string& op, const Client& source);
			void removeUser(const Client& user);
			std::vector<Client>& getUsers();

};

#endif
