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
		std::string					name;
		std::string					pwd;
		std::string					topic;
		std::vector<Client>			users;
		std::vector<std::string>	ops;
		std::vector<std::string>	invitedUsers;
		bool						invOnly;
		bool						topicSet;
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
			bool		isOp(const std::string& nickName) const;
			bool		isUserInChannel(const std::string &check) const;
			bool		getTopicSet() const;
			void		setTopicSet(const bool& topicSet);
			int			getMaxUsers() const;
			std::vector<std::string> &getOps();
			std::string getTopic() const;
			std::vector<std::string> getInvitedUsers() const;
			std::vector<Client>& getUsers();
			
			void addinvitedUser(const std::string& user);
			void addUser(const Client& user);
			void addOp(const std::string& op);
			void removeUser(const Client& user);
			void removeOp(const std::string& op);
};

#endif
