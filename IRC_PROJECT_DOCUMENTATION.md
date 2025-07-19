# IRC Server Project - Complete Technical Documentation

## Table of Contents
1. [Project Overview](#project-overview)
2. [Architecture Overview](#architecture-overview)
3. [Header Files Documentation](#header-files-documentation)
4. [Source Files Documentation](#source-files-documentation)
5. [Class Structure and Relationships](#class-structure-and-relationships)
6. [Protocol Implementation](#protocol-implementation)
7. [Error Handling](#error-handling)

---

## Project Overview

This IRC (Internet Relay Chat) server implementation is built in C++98 standard, providing a multi-client chat server that supports standard IRC protocol commands. The server uses non-blocking I/O with the `poll()` system call to handle multiple concurrent client connections efficiently.

### Key Features
- Multi-client support with non-blocking I/O
- Channel-based communication
- User authentication and registration
- Operator privileges system
- Bot integration
- Standard IRC command support (JOIN, PART, PRIVMSG, NICK, etc.)

---

## Architecture Overview

The project follows an object-oriented design with clear separation of concerns:

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     Server      │────│    Commands     │────│     Parser      │
│   (Network I/O) │    │  (IRC Protocol) │    │  (Message Parse)│
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     Client      │    │    Channel      │    │   Data Types    │
│  (User State)   │    │ (Chat Rooms)    │    │   (Structures)  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

---

## Header Files Documentation

### Server.hpp

This header defines the main `Server` class which manages the IRC server's core functionality.

#### Color Macros
```cpp
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define CYAN "\033[1;36m"
#define BLUE "\033[1;34m"
#define WHITE "\033[1;37m"
#define RED "\033[1;31m"
#define RESET "\033[0m"
```

**Purpose**: These macros provide colored terminal output for better user experience and debugging.

#### Server Class

```cpp
class Server
{
    private:
        int                            server_fd;
        std::string                    pwd;
        std::vector<struct pollfd>     fds;
        std::map<int, Client>          clients;
        std::map<std::string, Channel> channels;
        std::vector<std::string>       userList;
        std::vector<std::string>       nickList;
```

**Private Members**:
- `server_fd`: File descriptor for the server socket
- `pwd`: Server password for client authentication
- `fds`: Vector of pollfd structures for poll() system call
- `clients`: Map of file descriptors to Client objects
- `channels`: Map of channel names to Channel objects  
- `userList`: Vector of registered usernames (prevents duplicates)
- `nickList`: Vector of registered nicknames (prevents duplicates)

**Key Methods**:
- `Server(const std::string& port, const std::string& pwd)`: Constructor that initializes server
- `void run()`: Main server loop using poll() for I/O multiplexing
- `bool addUser(std::string& user)`: Adds username to server registry
- `bool addNick(std::string& nick)`: Adds nickname to server registry
- `void removeUser(std::string user)`: Removes username from registry
- `void removeNick(std::string nick)`: Removes nickname from registry

#### PollFdMatch Struct
```cpp
struct PollFdMatch
{
    int fdToRemove;
    PollFdMatch(int fd) : fdToRemove(fd) {}

    bool operator()(const struct pollfd& pfd) const
    {
        return pfd.fd == fdToRemove;
    }
};
```

**Purpose**: A function object (functor) used with STL algorithms to find and remove specific file descriptors from the polls vector. This follows the STL predicate pattern for efficient container operations.

### Client.hpp

Defines the `Client` class representing individual connected users.

```cpp
class Client
{
    private:
        int                         fd;
        std::string                 nickname;
        std::string                 username;
        std::string                 hostname;
        std::string                 realname;
        std::string                 servername;
        std::string                 buffer;
        std::string                 pwd;
        bool                        isAuth;
        std::vector<std::string>    joined_channels;
```

**Private Members**:
- `fd`: Client's socket file descriptor
- `nickname`: IRC nickname (display name)
- `username`: System username for identification
- `hostname`: Client's hostname (set to "server" by default)
- `realname`: Full name of the user
- `servername`: Name of the server user is connected to
- `buffer`: Incoming message buffer for incomplete messages
- `pwd`: Password provided by client
- `isAuth`: Authentication status flag
- `joined_channels`: List of channels the client has joined

**Key Methods**:
- `bool hasFullMessage(std::string& out)`: Checks if buffer contains complete IRC message (ending with \\r\\n)
- `void appendToBuffer(const std::string& buffer)`: Adds incoming data to message buffer
- `void joinChannel(const std::string& channel)`: Adds channel to user's joined channels list
- `void partChannel(const std::string& channel)`: Removes channel from user's joined channels list
- `bool isProvided() const`: Checks if user has provided required information (nickname, username, realname)

### Channel.hpp

Defines the `Channel` class representing IRC chat rooms.

```cpp
class Channel
{
    private:
        std::string                 name;
        std::string                 pwd;
        std::string                 topic;
        std::vector<Client>         users;
        std::vector<std::string>    ops;
        std::vector<std::string>    invitedUsers;
        bool                        invOnly;
        bool                        topicSet;
        int                         maxUsers;
```

**Private Members**:
- `name`: Channel name (e.g., "#general")
- `pwd`: Channel password (if password-protected)
- `topic`: Channel topic message
- `users`: Vector of Client objects currently in the channel
- `ops`: Vector of operator nicknames with administrative privileges
- `invitedUsers`: Vector of users invited to invite-only channels
- `invOnly`: Flag indicating if channel is invite-only
- `topicSet`: Flag indicating if topic has been set
- `maxUsers`: Maximum number of users allowed (-1 for unlimited)

**Key Methods**:
- `void addUser(const Client& user)`: Adds user to channel (prevents duplicates)
- `void removeUser(const Client& user)`: Removes user from channel
- `bool isOp(const std::string& nickName) const`: Checks if user has operator privileges
- `bool isUserInChannel(const std::string& check) const`: Checks if user is in channel
- `void addOp(const std::string& op)`: Grants operator privileges to user
- `void removeOp(const std::string& op)`: Removes operator privileges (auto-assigns new op if needed)

### Commands.hpp

Defines the `Commands` class responsible for IRC protocol command handling.

```cpp
class Commands
{
    private:
        std::map<int, Client>&             clients;
        std::map<std::string, Channel>&    channels;
        static const int                   BOT_FD;
        Server&                           server;
        bool                              botExists;
        
        typedef void (Commands::*CommandHandler)(const std::string&, Client&);
        std::map<std::string, CommandHandler> commandHandlers;
```

**Private Members**:
- `clients`: Reference to server's client map
- `channels`: Reference to server's channel map
- `BOT_FD`: Static constant (-1) used as file descriptor for server bot
- `server`: Reference to main server object
- `botExists`: Flag tracking if server bot has been created
- `CommandHandler`: Function pointer typedef for command handling methods
- `commandHandlers`: Map of command strings to handler function pointers

**Command Handler Methods**:
- `void handlePass(const std::string& message, Client& client)`: Handles PASS command (authentication)
- `void handleJoin(const std::string& channelName, Client& client)`: Handles JOIN command
- `void handlePrivmsg(const std::string& message, Client& sender)`: Handles PRIVMSG command
- `void handleUserCommand(const std::string& msg, Client& client)`: Handles USER command
- `void handleNickCommand(const std::string& nick, Client& client)`: Handles NICK command
- `void handleModeCommand(const std::string& msg, Client& client)`: Handles MODE command
- `void handlePartCommand(const std::string& msg, Client& client)`: Handles PART command
- `void handleQuitCommand(const std::string& msg, Client& client)`: Handles QUIT command
- `void handleTopicCommand(const std::string& msg, Client& client)`: Handles TOPIC command
- `void handleKickCommand(const std::string& msg, Client& client)`: Handles KICK command
- `void handleInviteCommand(const std::string& msg, Client& client)`: Handles INVITE command

### Parser.hpp

Defines parsing structures and the `Parser` class for IRC message parsing.

#### Data Structures

```cpp
struct userInfo
{
    std::string userName;
    std::string realName;
};
```
**Purpose**: Holds parsed user information from USER command.

```cpp
struct reciveMessage
{
    std::string target;
    std::string message;
};
```
**Purpose**: Holds parsed private message information (target and message content).

```cpp
struct parseInfo
{
    std::string command;
    std::string function;
    std::string value;
};
```
**Purpose**: General structure for parsed IRC commands (command, primary parameter, additional parameters).

```cpp
struct modeInfo
{
    std::string channel;
    bool status;
    std::string key;
    std::string parameters;
};
```
**Purpose**: Holds parsed MODE command information (channel, add/remove flag, mode character, parameters).

#### Parser Class Methods

- `static reciveMessage privateMessage(std::string message)`: Parses PRIVMSG commands
- `static parseInfo parse(std::string message)`: General IRC message parser
- `static userInfo userParse(std::string message)`: Specialized parser for USER commands
- `static modeInfo modeParse(std::string message)`: Specialized parser for MODE commands

#### Utility Functions
- `std::vector<std::string> split(const std::string& s)`: Splits string by whitespace
- `std::string trim(const std::string& str)`: Removes leading/trailing whitespace

---

## Source Files Documentation

### main.cpp

The entry point of the IRC server application.

```cpp
void handleSignals(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
        throw std::runtime_error(RED"Server terminated by signal " + ft_itoa(signal) + RESET);
}
```

**Function**: `handleSignals(int signal)`
**Purpose**: Signal handler for graceful shutdown on SIGINT (Ctrl+C) and SIGTERM
**Parameters**: `signal` - The signal number received
**Behavior**: Throws a runtime_error exception to trigger graceful cleanup through exception handling

```cpp
int main(int ac, char **av)
{
    try
    {
        if (ac != 3)
            throw std::invalid_argument(RED"Wrong inputs.\n" GREEN"Usage: " WHITE"\"" + std::string(av[0]) + "\" \"port\" \"password\"" RESET);
        signal(SIGINT, handleSignals);
        signal(SIGTERM, handleSignals);
        Server server(av[1], av[2]);
        server.run();
        return (0);
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }
}
```

**Function**: `main(int ac, char **av)`
**Purpose**: Program entry point, validates arguments and starts server
**Parameters**: 
- `ac` - Argument count (must be 3)
- `av` - Argument vector [program_name, port, password]
**Error Handling**: Uses exception-based error handling for clean resource management
**Flow**: Validates arguments → Sets up signal handlers → Creates and runs server → Handles exceptions

### Server.cpp

#### Constructor and Destructor

```cpp
Server::Server(const std::string& port, const std::string& pwd)
{
    if (!checkPort(port))
        throw std::invalid_argument(RED"Invalid port.\n" GREEN"Usage: " WHITE"\"./ft_irc\" \"0 < port < 65536\" \"password\"" RESET);
    this->pwd = pwd;
    initServer(port);
}
```

**Function**: `Server::Server(const std::string& port, const std::string& pwd)`
**Purpose**: Initializes IRC server with specified port and password
**Parameters**:
- `port` - Port number as string (validated to be 1-65535)
- `pwd` - Server password for client authentication
**Validation**: Calls `checkPort()` to ensure port is valid integer in correct range
**Initialization**: Sets password and calls `initServer()` to set up socket

```cpp
Server::~Server()
{
    for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
        close(it->first);
    close(server_fd);
    clients.clear();
    fds.clear();
    channels.clear();
}
```

**Function**: `Server::~Server()`
**Purpose**: Cleans up all resources when server shuts down
**Cleanup Process**:
1. Closes all client socket file descriptors
2. Closes server socket file descriptor
3. Clears all containers (clients, fds, channels)
**Resource Safety**: Ensures no file descriptor leaks on shutdown

#### Core Server Methods

```cpp
void Server::run()
{
    while (true)
    {
        int ready = poll(fds.data(), fds.size(), -1);
        
        if (ready == -1)
            throw std::runtime_error(RED"Error during poll: " + std::string(strerror(errno)) + RESET);
            
        // Handle new connections
        if (fds[0].revents & POLLIN)
        {
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd == -1)
                throw std::runtime_error(RED"Error accepting new client: " + std::string(strerror(errno)) + RESET);
                
            if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
                throw std::runtime_error(RED"Error setting non-blocking mode: " + std::string(strerror(errno)) + RESET);
                
            clients.insert(std::make_pair(client_fd, Client(client_fd)));
            
            struct pollfd client_pollfd;
            client_pollfd.fd = client_fd;
            client_pollfd.events = POLLIN;
            fds.push_back(client_pollfd);
            
            std::cout << "New client connected: fd = " << client_fd << std::endl;
            ready--;
        }
        
        // Handle existing client data
        for (size_t i = 1; i < fds.size() && ready > 0; ++i)
        {
            // ... handle client I/O
        }
    }
}
```

**Function**: `Server::run()`
**Purpose**: Main server event loop using poll() for I/O multiplexing
**Algorithm**:
1. **Polling**: Uses `poll()` system call to wait for I/O events on multiple file descriptors
2. **New Connections**: When server socket has incoming connection (POLLIN on fds[0]):
   - Accepts new connection with `accept()`
   - Sets socket to non-blocking mode with `fcntl()`
   - Creates new Client object and adds to clients map
   - Adds new pollfd structure to fds vector
3. **Client Data**: For each existing client socket with POLLIN event:
   - Reads data into buffer
   - Appends to client's message buffer
   - Processes complete IRC messages
4. **Error Handling**: Handles POLLHUP/POLLERR events for disconnected clients

**Poll System Call Details**:
- `poll(fds.data(), fds.size(), -1)`: Monitors all file descriptors indefinitely
- Returns number of file descriptors with events ready
- Events monitored: POLLIN (data available for reading)

#### Client I/O Handling

```cpp
if (fds[i].revents & POLLIN)
{
    char buffer[1024];
    ssize_t n;
    while ((n = read(fds[i].fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[n] = '\0';
        std::map<int, Client>::iterator it = clients.find(fds[i].fd);
        if (it == clients.end())
        {
            std::cerr << "Error: Client not found for fd " << fds[i].fd << std::endl;
            break;
        }
        
        Client& client = it->second;
        client.appendToBuffer(buffer);
        client.setPwd(pwd);
        
        std::string message;
        while (client.hasFullMessage(message))
        {
            handleClientMessage(client, message);
            std::cout << "IRC message from {" << fds[i].fd << "} : [" << message << "]" << std::endl;
        }
    }
}
```

**Purpose**: Handles incoming data from client sockets
**Process**:
1. **Reading**: Uses `read()` in a loop to get all available data
2. **Buffering**: Appends data to client's internal buffer
3. **Message Processing**: Checks for complete IRC messages (ending with \\r\\n)
4. **Command Execution**: Calls `handleClientMessage()` for each complete message
5. **Safety Check**: Verifies client exists in map before accessing (prevents map::at exceptions)

#### Client Disconnection Handling

```cpp
if (n == 0)
{
    std::cout << "Client disconnected: fd = " << fds[i].fd << std::endl;
    std::map<int, Client>::iterator it = clients.find(fds[i].fd);
    if (it != clients.end())
    {
        removeUser(it->second.getUsername());
        removeNick(it->second.getNickname());
        std::string quit = "QUIT\r\n";
        handleClientMessage(it->second, quit);
        clients.erase(it);
    }
    
    close(fds[i].fd);
    fds.erase(fds.begin() + i);
    --i;
}
```

**Purpose**: Handles client disconnections gracefully
**Cleanup Process**:
1. **User Registry**: Removes username and nickname from server lists
2. **QUIT Processing**: Sends QUIT command to handle channel departures
3. **Client Removal**: Removes client from clients map
4. **Socket Cleanup**: Closes file descriptor and removes from polls vector
5. **Iterator Safety**: Decrements loop index to account for vector element removal

#### Utility Methods

```cpp
bool Server::checkPort(const std::string& port)
{
    for (size_t i = 0; i < port.size(); i++)
        if (!std::isdigit(port[i]))
            return (false);
            
    if (port.size() > 5)
        return (false);
    else if (std::strtol(port.c_str(), NULL, 10) <= 0 || std::strtol(port.c_str(), NULL, 10) > 65535)
        return (false);
        
    return (true);
}
```

**Function**: `Server::checkPort(const std::string& port)`
**Purpose**: Validates port number string
**Validation Rules**:
1. All characters must be digits
2. Maximum 5 characters (65535 is max valid port)
3. Numeric value must be 1-65535
**Returns**: `true` if valid, `false` otherwise

```cpp
void Server::initServer(const std::string& port)
{
    int opt;
    struct sockaddr_in address;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        throw std::runtime_error(RED"Error: socket creation failed " + std::string(strerror(errno)) + RESET);
        
    opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(server_fd);
        throw std::runtime_error(RED"Error: setsockopt failed " + std::string(strerror(errno)) + RESET);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(std::strtol(port.c_str(), NULL, 10));
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        close(server_fd);
        throw std::runtime_error(RED"Error: bind failed " + std::string(strerror(errno)) + RESET);
    }
    
    if (listen(server_fd, 10) < 0)
    {
        close(server_fd);
        throw std::runtime_error(RED"Error: listen failed " + std::string(strerror(errno)) + RESET);
    }
    
    std::cout << BLUE"Server is running on port " << port << RESET << std::endl;
    
    struct pollfd server_pollfd;
    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    fds.push_back(server_pollfd);
}
```

**Function**: `Server::initServer(const std::string& port)`
**Purpose**: Initializes server socket and sets up networking
**Socket Setup Process**:
1. **Socket Creation**: Creates TCP socket with `socket(AF_INET, SOCK_STREAM, 0)`
2. **Socket Options**: Sets SO_REUSEADDR to allow port reuse after server restart
3. **Address Binding**: Binds socket to specified port on all interfaces (INADDR_ANY)
4. **Listen Queue**: Sets up listen queue with backlog of 10 connections
5. **Poll Setup**: Adds server socket to poll file descriptor list

**Error Handling**: Each socket operation checks for errors and throws exceptions with cleanup

### Client.cpp

#### Constructor and Basic Methods

```cpp
Client::Client(const int& fd)
{
    this->fd = fd;
    this->isAuth = false;
    this->hostname = "server";
}
```

**Function**: `Client::Client(const int& fd)`
**Purpose**: Initializes new client with file descriptor
**Initial State**:
- Sets file descriptor
- Authentication status set to false
- Hostname set to default "server"
- All string fields empty until set by IRC commands

#### Message Buffer Management

```cpp
bool Client::hasFullMessage(std::string& out)
{
    size_t pos = buffer.find("\r\n");
    if (pos == std::string::npos)
        return false;
        
    out = buffer.substr(0, pos + 2);
    buffer.erase(0, pos + 2);
    return true;
}
```

**Function**: `Client::hasFullMessage(std::string& out)`
**Purpose**: Checks if client buffer contains complete IRC message
**IRC Protocol**: Messages end with \\r\\n (carriage return + line feed)
**Parameters**: `out` - Reference to string that will receive the complete message
**Returns**: `true` if complete message found, `false` otherwise
**Side Effect**: Removes the extracted message from internal buffer
**Buffer Management**: Uses sliding window approach to handle partial messages

```cpp
void Client::appendToBuffer(const std::string& data)
{
    buffer += data;
}
```

**Function**: `Client::appendToBuffer(const std::string& data)`
**Purpose**: Adds incoming network data to message buffer
**Use Case**: TCP may deliver data in chunks; this accumulates partial messages
**Buffer Growth**: Buffer grows until complete message (with \\r\\n) is received

#### Channel Management

```cpp
void Client::joinChannel(const std::string& channel)
{
    if (std::find(joined_channels.begin(), joined_channels.end(), channel) == joined_channels.end())
        joined_channels.push_back(channel);
}
```

**Function**: `Client::joinChannel(const std::string& channel)`
**Purpose**: Adds channel to client's joined channels list
**Duplicate Prevention**: Checks if channel already in list before adding
**Use Case**: Called when client successfully joins a channel

```cpp
void Client::partChannel(const std::string& channel)
{
    std::vector<std::string>::iterator it = std::find(joined_channels.begin(), joined_channels.end(), channel);
    if (it != joined_channels.end())
        joined_channels.erase(it);
}
```

**Function**: `Client::partChannel(const std::string& channel)`
**Purpose**: Removes channel from client's joined channels list
**Safe Removal**: Only removes if channel exists in list
**Use Case**: Called when client leaves a channel (PART command)

#### Client State Validation

```cpp
bool Client::isProvided() const
{
    return !this->nickname.empty() && !this->username.empty() && !this->realname.empty();
}
```

**Function**: `Client::isProvided() const`
**Purpose**: Checks if client has provided all required registration information
**IRC Registration**: Clients must provide nickname, username, and realname before joining channels
**Returns**: `true` if all required fields are set, `false` otherwise
**Use Case**: Server checks this before allowing channel operations

### Channel.cpp

#### Constructor and Initialization

```cpp
Channel::Channel()
{
    this->name = "";
    this->pwd = "";
    this->invOnly = false;
    this->maxUsers = -1;
    this->topic = "The topic has not been set yet.";
    this->topicSet = false;
}

Channel::Channel(const std::string& name)
{
    this->name = name;
    this->pwd = "";
    this->invOnly = false;
    this->maxUsers = -1;
    this->topic = "The topic has not been set yet.";
    this->topicSet = false;
}
```

**Purpose**: Initializes channel with default settings
**Default State**:
- No password protection
- Not invite-only
- Unlimited users (maxUsers = -1)
- Default topic message
- Topic not set by user

#### User Management

```cpp
void Channel::addUser(const Client& user)
{
    for (std::vector<Client>::iterator it = users.begin(); it != users.end(); ++it)
        if (it->getFd() == user.getFd())
            return;
    users.push_back(user);
}
```

**Function**: `Channel::addUser(const Client& user)`
**Purpose**: Adds user to channel user list
**Duplicate Prevention**: Checks file descriptor to prevent duplicate additions
**Client Identification**: Uses file descriptor as unique identifier
**Storage**: Stores complete Client object (not just reference)

```cpp
void Channel::removeUser(const Client& user)
{
    std::vector<Client>::iterator it = users.begin();
    std::vector<Client>::iterator ite = users.end();
    while (it != ite)
    {
        if (it->getFd() == user.getFd())
        {
            users.erase(it);
            return;
        }
        ++it;
    }
}
```

**Function**: `Channel::removeUser(const Client& user)`
**Purpose**: Removes user from channel user list
**Search Method**: Linear search by file descriptor
**Safe Removal**: Only removes if user found in list
**Iterator Safety**: Proper iterator handling to avoid invalidation

#### Operator Management

```cpp
void Channel::removeOp(const std::string& op)
{
    std::vector<std::string>::iterator it = std::find(ops.begin(), ops.end(), op);
    if (it == ops.end())
        return;
        
    ops.erase(it);
    if (!ops.empty())
        return;
        
    std::vector<Client> newlist = users;
    for (std::vector<Client>::iterator iter = newlist.begin(); iter != newlist.end(); ++iter)
    {
        if (iter->getNickname() == op)
        {
            newlist.erase(iter);
            break;
        }
    }
    
    if (newlist.empty())
        return;
        
    Client& newOpClient = newlist[rand() % newlist.size()];
    std::string newOp = newOpClient.getNickname();
    ops.push_back(newOp);
    
    std::string ModeMsg = ":Server MODE " + this->name + " +o " + newOp + "\r\n";
    for (std::vector<Client>::iterator user = users.begin(); user != users.end(); ++user)
        send(user->getFd(), ModeMsg.c_str(), ModeMsg.size(), 0);
}
```

**Function**: `Channel::removeOp(const std::string& op)`
**Purpose**: Removes operator privileges and auto-assigns new operator if needed
**Auto-Assignment Logic**:
1. Remove specified operator from ops list
2. If no operators remain, select random user from remaining users
3. Grant operator privileges to selected user
4. Send MODE message to all channel users announcing new operator
**Channel Continuity**: Ensures channel always has at least one operator if users present

### Commands.cpp

#### Command System Architecture

```cpp
Commands::Commands(std::map<int, Client>& c, std::map<std::string, Channel>& ch, Server& server)
    : clients(c), channels(ch), server(server), botExists(false)
{
    initializeCommandHandlers();
}

void Commands::initializeCommandHandlers()
{
    commandHandlers["PASS"] = &Commands::handlePass;
    commandHandlers["JOIN"] = &Commands::handleJoin;
    commandHandlers["PRIVMSG"] = &Commands::handlePrivmsg;
    commandHandlers["USER"] = &Commands::handleUserCommand;
    commandHandlers["NICK"] = &Commands::handleNickCommand;
    commandHandlers["MODE"] = &Commands::handleModeCommand;
    commandHandlers["PART"] = &Commands::handlePartCommand;
    commandHandlers["QUIT"] = &Commands::handleQuitCommand;
    commandHandlers["TOPIC"] = &Commands::handleTopicCommand;
    commandHandlers["KICK"] = &Commands::handleKickCommand;
    commandHandlers["INVITE"] = &Commands::handleInviteCommand;
}
```

**Purpose**: Implements command dispatch pattern using function pointers
**Architecture Benefits**:
- Extensible: Easy to add new commands
- Maintainable: Each command has dedicated handler
- Efficient: O(1) command lookup using map
**Function Pointer Usage**: Uses member function pointers for type-safe dispatch

#### Main Command Dispatcher

```cpp
void Commands::executeCommand(const std::string& raw, Client& client)
{
    parseInfo info = Parser::parse(raw);
    std::string cmd = info.command;
    
    if (cmd == "PASS")
    {
        handlePass(raw, client);
        return;
    }
    
    if (!client.getIsAuth())
    {
        std::string err = "You have not registered yet\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
    
    if (!client.isProvided())
    {
        if (cmd == "USER")
            handleUserCommand(raw, client);
        else if (cmd == "NICK")
            handleNickCommand(raw, client);
        else
        {
            std::string err = "Please provide your nickname and username by using /NICK <nickname> and /USER <'username' 0 * :'realname'>\r\n";
            send(client.getFd(), err.c_str(), err.size(), 0);
        }
        return;
    }
    
    std::map<std::string, CommandHandler>::iterator it = commandHandlers.find(cmd);
    if (it != commandHandlers.end())
        (this->*(it->second))(raw, client);
    else
    {
        std::string err = ":server 421 " + client.getNickname() + " " + cmd + " :Unknown command\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
    }
}
```

**Function**: `Commands::executeCommand(const std::string& raw, Client& client)`
**Purpose**: Main command dispatcher with authentication and state validation
**Flow Control**:
1. **Parse Command**: Extract command type from raw IRC message
2. **Special Case**: PASS command handled before authentication check
3. **Authentication Check**: Verify client has provided valid password
4. **Registration Check**: Verify client has provided required information
5. **Command Dispatch**: Look up and execute appropriate handler
6. **Error Handling**: Send appropriate error messages for unknown commands

#### Authentication Commands

```cpp
void Commands::handlePass(const std::string& message, Client& client)
{
    parseInfo info = Parser::parse(message);
    
    if (client.getIsAuth())
    {
        std::string err = "You may not reregister\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
    
    if (info.function.empty())
    {
        std::string err = "Not enough parameters for PASS. Use correct format: '/PASS <pwd>'\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
    
    if (info.function != client.getPwd())
    {
        std::string err = "Password is incorrect\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
    
    client.setIsAuth(true);
    std::string success = "Welcome to the Concord. You are now registered.\r\n";
    send(client.getFd(), success.c_str(), success.size(), 0);
}
```

**Function**: `Commands::handlePass(const std::string& message, Client& client)`
**Purpose**: Handles PASS command for client authentication
**IRC Protocol**: PASS must be first command sent by client
**Validation**:
1. Prevents re-authentication of already authenticated clients
2. Validates password parameter is provided
3. Compares provided password with server password
**State Change**: Sets client authentication status to true on success

```cpp
void Commands::handleUserCommand(const std::string& msg, Client& client)
{
    userInfo info = Parser::userParse(msg);
    if (info.userName.empty() || info.realName.empty())
    {
        std::string err = "Not enough parameters for USER. Use the correct format: '/USER <username> 0 * :<realname>'\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
    
    if (info.userName == "bot")
    {
        std::string err = "Username 'bot' is reserved for the server bot\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
    
    if (server.addUser(info.userName) == false)
    {
        std::string err = "Username already exists. Please choose a different username.\r\n";
        send(client.getFd(), err.c_str(), err.size(), 0);
        return;
    }
    
    std::string oldUsername = client.getUsername();
    if (!oldUsername.empty())
        server.removeUser(oldUsername);
    
    client.setUsername(info.userName);
    client.setRealname(info.realName);
    std::string noticeMsg = "Your username is set to: " + info.userName + "\r\n";
    send(client.getFd(), noticeMsg.c_str(), noticeMsg.size(), 0);
    std::string noticeMsg2 = "Your realname is set to: " + info.realName + "\r\n";
    send(client.getFd(), noticeMsg2.c_str(), noticeMsg2.size(), 0);
}
```

**Function**: `Commands::handleUserCommand(const std::string& msg, Client& client)`
**Purpose**: Handles USER command for client registration
**IRC Format**: `USER <username> <hostname> <servername> :<realname>`
**Validation**:
1. Ensures username and realname are provided
2. Prevents use of reserved "bot" username
3. Checks for username uniqueness across server
**State Management**: Updates server's user registry and client state

### Parser.cpp

#### String Utility Functions

```cpp
std::vector<std::string> split(const std::string& s)
{
    std::vector<std::string> tokens;
    
    size_t start = 0, end = 0;
    while (start < s.length())
    {
        while (start < s.length() && std::isspace(s[start])) ++start;
        if (start >= s.length()) break;
        
        end = start;
        while (end < s.length() && !std::isspace(s[end])) ++end;
        
        tokens.push_back(s.substr(start, end - start));
        start = end;
    }
    return tokens;
}
```

**Function**: `split(const std::string& s)`
**Purpose**: Splits string into tokens separated by whitespace
**Algorithm**:
1. Skip leading whitespace
2. Find end of current token (next whitespace)
3. Extract token and add to vector
4. Repeat until end of string
**Robustness**: Handles multiple consecutive spaces and leading/trailing whitespace

```cpp
std::string trim(const std::string& str)
{
    size_t start = 0;
    size_t end = str.size() - 1;
    
    while (start <= end && std::isspace(str[start]))
        ++start;
        
    while (end >= start && std::isspace(str[end]))
        --end;
        
    return str.substr(start, end - start + 1);
}
```

**Function**: `trim(const std::string& str)`
**Purpose**: Removes leading and trailing whitespace from string
**Algorithm**: Find first and last non-whitespace characters, return substring
**Edge Cases**: Handles empty strings and strings with only whitespace

#### IRC Message Parsers

```cpp
parseInfo Parser::parse(std::string message)
{
    parseInfo info;
    
    if (message.find(" ") == std::string::npos)
    {
        info.command = message;
        return info;
    }
    
    std::vector<std::string> words = split(message);
    
    info.command = trim(words[0]);
    if (words.size() == 1)
        return info;
        
    info.function = trim(words[1]);
    if (words.size() == 2)
    {
        info.value = "";
        return info;
    }
    
    for (size_t i = 2; i < words.size(); i++)
    {
        info.value += trim(words[i]);
        if (i != words.size() - 1)
            info.value += " ";
    }
    
    return info;
}
```

**Function**: `Parser::parse(std::string message)`
**Purpose**: General IRC message parser extracting command and parameters
**Structure**: Handles IRC format: `COMMAND parameter1 parameter2 ...`
**Components**:
- `command`: The IRC command (JOIN, PRIVMSG, etc.)
- `function`: First parameter (often target)
- `value`: Remaining parameters concatenated with spaces

```cpp
reciveMessage Parser::privateMessage(std::string message)
{
    reciveMessage info;
    
    std::vector<std::string> words = split(message);
    if (words.size() < 3)
    {
        info.target = "";
        info.message = "";
        return info;
    }
    
    info.target = words[1];
    
    if (words[2][0] == ':')
        words[2].erase(0, 1);
        
    for (size_t i = 2; i < words.size(); i++)
    {
        info.message += trim(words[i]);
        if (i != words.size() - 1)
            info.message += " ";
    }
    
    return info;
}
```

**Function**: `Parser::privateMessage(std::string message)`
**Purpose**: Specialized parser for PRIVMSG commands
**IRC Format**: `PRIVMSG <target> :<message>`
**Special Handling**: Removes leading ':' from message content (IRC protocol requirement)
**Validation**: Ensures minimum required parameters are present

---

## Class Structure and Relationships

### Dependency Graph

```
main.cpp
    │
    ▼
Server.cpp ──────┐
    │            │
    ▼            ▼
Client.cpp   Commands.cpp ──────┐
    │            │              │
    │            ▼              ▼
    │        Channel.cpp    Parser.cpp
    │            │
    └────────────┘
```

### Data Flow

1. **main.cpp**: Entry point, creates Server instance
2. **Server**: Manages network I/O, maintains client/channel collections
3. **Client**: Represents individual user state and message buffering
4. **Commands**: Processes IRC protocol commands, updates Client/Channel state
5. **Channel**: Manages chat room state and user lists
6. **Parser**: Converts raw IRC messages into structured data

### Key Interactions

- **Server ↔ Client**: Server manages Client lifecycle and message routing
- **Commands ↔ Parser**: Commands uses Parser to interpret IRC messages
- **Commands ↔ Channel**: Commands modifies Channel state based on user actions
- **Commands ↔ Server**: Commands queries/updates server-wide user/nick registries

---

## Protocol Implementation

### IRC Command Support

| Command | Purpose | Parameters | Implementation |
|---------|---------|------------|----------------|
| PASS | Authentication | `<password>` | `Commands::handlePass()` |
| NICK | Set nickname | `<nickname>` | `Commands::handleNickCommand()` |
| USER | User registration | `<username> 0 * :<realname>` | `Commands::handleUserCommand()` |
| JOIN | Join channel | `<channel>` | `Commands::handleJoin()` |
| PART | Leave channel | `<channel> [:<reason>]` | `Commands::handlePartCommand()` |
| PRIVMSG | Send message | `<target> :<message>` | `Commands::handlePrivmsg()` |
| QUIT | Disconnect | `[:<reason>]` | `Commands::handleQuitCommand()` |
| MODE | Change modes | `<target> <modes> [<parameters>]` | `Commands::handleModeCommand()` |
| TOPIC | Set/view topic | `<channel> [:<topic>]` | `Commands::handleTopicCommand()` |
| KICK | Remove user | `<channel> <user> [:<reason>]` | `Commands::handleKickCommand()` |
| INVITE | Invite user | `<nickname> <channel>` | `Commands::handleInviteCommand()` |

### Message Format

IRC messages follow the format: `COMMAND [parameters] \r\n`

**Examples**:
- `PASS mypassword\r\n`
- `NICK JohnDoe\r\n`
- `JOIN #general\r\n`
- `PRIVMSG #general :Hello everyone!\r\n`

### Authentication Flow

1. **Connection**: Client connects to server
2. **PASS**: Client sends password for authentication
3. **NICK**: Client sets nickname
4. **USER**: Client provides user information
5. **Ready**: Client can now join channels and send messages

### Channel Operations

1. **JOIN**: Add user to channel, create channel if doesn't exist
2. **PRIVMSG**: Send message to all users in channel
3. **PART**: Remove user from channel
4. **MODE**: Change channel settings (invite-only, password, etc.)
5. **KICK**: Operator removes another user from channel

---

## Error Handling

### Exception-Based Error Handling

The server uses C++ exceptions for error handling:

```cpp
try
{
    Server server(av[1], av[2]);
    server.run();
}
catch(const std::exception &e)
{
    std::cerr << e.what() << std::endl;
    return (1);
}
```

### Error Categories

1. **System Errors**: Socket operations, file descriptor operations
2. **Protocol Errors**: Invalid IRC commands, missing parameters
3. **Authentication Errors**: Invalid passwords, registration issues
4. **Resource Errors**: Channel limits, duplicate nicknames/usernames

### Error Response Format

IRC errors follow standard numeric reply format:
```
:server <error_code> <nickname> <parameters> :<description>
```

Example: `:server 421 JohnDoe INVALID :Unknown command`

### Resource Management

- **RAII**: Constructors acquire resources, destructors release them
- **Exception Safety**: All socket operations checked and cleaned up on error
- **Memory Management**: STL containers handle memory automatically
- **File Descriptor Management**: Explicit close() calls in destructors and error paths

---

## Conclusion

This IRC server implementation demonstrates solid software engineering principles:

- **Modularity**: Clear separation of concerns across classes
- **Extensibility**: Command system easily accommodates new IRC commands
- **Robustness**: Comprehensive error handling and resource management
- **Performance**: Efficient I/O multiplexing with poll() system call
- **Standards Compliance**: Follows IRC protocol specifications

The codebase serves as an excellent foundation for understanding network programming, protocol implementation, and object-oriented design in C++98.
