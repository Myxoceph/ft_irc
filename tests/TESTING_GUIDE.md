# ft_IRC Manual Testing Guide

This document provides manual test cases you can perform to verify your IRC server implementation.

## Prerequisites

1. Compile your server: `make`
2. Start your server: `./ft_irc <port> <password>`
3. Use an IRC client like `nc` (netcat), `telnet`, or a GUI IRC client

## Test Cases

### 1. Basic Connection and Authentication

**Using netcat:**
```bash
nc localhost 6667
```

**Commands to test:**
```irc
PASS test123
NICK testuser
USER testuser 0 * :Test User
```

**Expected results:**
- Server should accept password
- Nickname should be set
- Welcome messages should be received

### 2. Channel Operations

**Test joining channels:**
```irc
JOIN #general
JOIN #test
```

**Test channel messaging:**
```irc
PRIVMSG #general :Hello everyone!
PRIVMSG #test :Testing channel messages
```

**Test leaving channels:**
```irc
PART #test
```

### 3. Private Messages

**Test private messaging:**
```irc
PRIVMSG testuser2 :This is a private message
```

### 4. Channel Modes (Operator Commands)

**Test invite-only mode:**
```irc
MODE #general +i
MODE #general -i
```

**Test channel password:**
```irc
MODE #general +k secret123
MODE #general -k
```

**Test user limit:**
```irc
MODE #general +l 10
MODE #general -l
```

**Test operator privileges:**
```irc
MODE #general +o username
MODE #general -o username
```

### 5. Topic Management

**Test setting topic:**
```irc
TOPIC #general :Welcome to the general channel
```

**Test viewing topic:**
```irc
TOPIC #general
```

### 6. Operator Commands

**Test kicking users (requires operator privileges):**
```irc
KICK #general username :Reason for kick
```

**Test inviting users:**
```irc
INVITE username #general
```

### 7. Error Conditions

**Test invalid commands:**
```irc
INVALIDCOMMAND
PRIVMSG
JOIN
```

**Test non-existent targets:**
```irc
PRIVMSG nonexistentuser :This should fail
KICK #general nonexistentuser
```

**Test permission errors:**
```irc
MODE #general +o someuser
# (when not an operator)
```

### 8. Multiple Client Testing

Open multiple terminals and connect different clients to test:
- Multiple users in the same channel
- Private messages between users
- Operator commands affecting other users
- Channel mode changes affecting all users

### 9. Disconnection Testing

**Test graceful quit:**
```irc
QUIT :Goodbye!
```

**Test ungraceful disconnect:**
- Close terminal/connection without QUIT command

## Expected Server Behavior

1. **Authentication**: Must authenticate with PASS before other commands
2. **Registration**: Must provide both NICK and USER before joining channels
3. **Channel Management**: First user to join becomes operator
4. **Error Handling**: Appropriate error codes for invalid commands/parameters
5. **Message Broadcasting**: Messages sent to channels reach all members
6. **Operator Privileges**: Only operators can use MODE, KICK, INVITE, TOPIC commands
7. **Clean Disconnection**: Server handles client disconnections gracefully

## IRC Error Codes to Test

- `461`: Not enough parameters
- `401`: No such nick/channel
- `403`: No such channel
- `431`: No nickname given
- `441`: They aren't on that channel
- `442`: You're not on that channel
- `443`: User is already on channel
- `451`: You have not registered
- `462`: You may not reregister
- `464`: Password incorrect
- `471`: Cannot join channel (+l)
- `473`: Cannot join channel (+i)
- `481`: Permission Denied
- `482`: You're not channel operator

## Stress Testing

1. **Multiple simultaneous connections**
2. **Rapid command sending**
3. **Large messages**
4. **Many channels with many users**
5. **Frequent join/part operations**

## Using the Automated Test Scripts

### Comprehensive Test Suite
```bash
./test_irc.sh
```
This runs a full suite of tests with multiple bots.

### Quick Test
```bash
./quick_test.sh [port] [password]
```
This runs a basic functionality test.

### Custom Port/Password
```bash
./quick_test.sh 8080 mypassword
```

## Tips for Manual Testing

1. **Use multiple terminals** to simulate multiple users
2. **Test edge cases** like empty parameters, invalid channels, etc.
3. **Verify error messages** are appropriate and helpful
4. **Check server logs** for proper handling of all scenarios
5. **Test with real IRC clients** like HexChat, WeeChat, or irssi for compatibility

Remember: The key to good testing is to try breaking your server in creative ways!
