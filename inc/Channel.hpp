#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <sys/socket.h>

class Client;

class Channel
{
	private:
		// Channel attributes
		std::string					name;
		std::string					pwd;
		std::string					topic;
		bool						invOnly;
		bool						topicRestricted; // For mode +t
		int							maxUsers;

		// User lists
		std::vector<Client*>		users;
		std::vector<Client*>		ops;
		std::vector<std::string>	invitedUsers;

	public:
		// Constructors and Destructors
		Channel();
		Channel(const std::string& name);
		~Channel();

		// Getters
		std::string	getName() const;
		std::string	getPwd() const;
		std::string getTopic() const;
		bool		getInvOnly() const;
		bool		getTopicRestricted() const;
		int			getMaxUsers() const;
		std::vector<Client*>& getUsers();
		std::vector<Client*>& getOps();
		std::vector<std::string> getInvitedUsers() const;

		// Setters
		void setName(const std::string& name);
		void setPwd(const std::string& pwd);
		void setTopic(const std::string& topic);
		void setInvOnly(const bool& invOnly);
		void setTopicRestricted(const bool& topicRestricted);
		void setMaxUsers(const int& maxUsers);

		// User management
		void addUser(Client* user);
		void removeUser(Client* user);
		void addOp(Client* user);
		void removeOp(Client* user);
		void addInvitedUser(const std::string& user);
		bool isOp(Client* user) const;
		bool isUserInChannel(const std::string& nickname) const;
		bool isInvited(const std::string& nickname) const;

		// Communication
		void broadcast(const std::string& message);
		void broadcast(const std::string& message, Client* exclude);
};

#endif
