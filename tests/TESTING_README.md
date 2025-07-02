# ft_IRC Testing Scripts

This directory contains several testing scripts to thoroughly test your IRC server implementation.

## Quick Start

1. **Compile your server:**
   ```bash
   make
   ```

2. **Start your server:**
   ```bash
   ./ft_irc 6667 test123
   ```

3. **Run tests:**
   ```bash
   ./quick_test.sh        # Basic functionality test
   ./test_irc.sh          # Comprehensive test suite
   ./stress_test.sh       # Performance/stress testing
   ./irc_client.sh        # Interactive IRC client
   ```

## Testing Scripts Overview

### 1. `quick_test.sh` - Basic Functionality Test
- **Purpose**: Quick verification that basic IRC functionality works
- **Usage**: `./quick_test.sh [port] [password]`
- **Tests**: Authentication, registration, channel join, messaging
- **Duration**: ~10 seconds

### 2. `test_irc.sh` - Comprehensive Test Suite
- **Purpose**: Complete testing of all IRC features
- **Usage**: `./test_irc.sh`
- **Tests**:
  - Authentication and registration
  - Multiple users and channels
  - Channel modes (+i, +k, +l, +o, +t)
  - Operator commands (KICK, INVITE)
  - Private messaging
  - Error conditions
  - Invalid commands
- **Features**: Creates 12+ bot clients that interact with each other
- **Duration**: ~30 seconds

### 3. `stress_test.sh` - Performance Testing
- **Purpose**: Test server performance under load
- **Usage**: `./stress_test.sh [port] [password] [num_clients]`
- **Default**: 10 concurrent clients
- **Tests**: Multiple simultaneous connections, rapid messaging, channel operations
- **Duration**: ~15 seconds

### 4. `irc_client.sh` - Interactive IRC Client
- **Purpose**: Manual testing with a simple IRC client
- **Usage**: `./irc_client.sh [port] [password] [nickname]`
- **Features**: Interactive command line IRC client for manual testing
- **Commands**: All standard IRC commands (JOIN, PRIVMSG, MODE, etc.)

## Test Scenarios Covered

### ✅ Authentication & Registration
- PASS command validation
- NICK command (nickname setting)
- USER command (user registration)
- Authentication failure handling

### ✅ Channel Operations
- JOIN (creating and joining channels)
- PART (leaving channels)
- Multiple channel membership
- Channel creator becomes operator

### ✅ Messaging
- PRIVMSG to channels
- PRIVMSG to individual users
- Message broadcasting to channel members
- Private message delivery

### ✅ Channel Modes
- `+i` Invite-only mode
- `+k` Channel password/key
- `+l` User limit
- `+o` Operator privileges
- `+t` Topic restriction

### ✅ Operator Commands
- KICK (removing users from channels)
- INVITE (inviting users to invite-only channels)
- TOPIC (setting/viewing channel topics)
- MODE (changing channel modes)

### ✅ Error Handling
- Invalid commands
- Missing parameters
- Permission errors
- Non-existent users/channels
- Authentication failures

### ✅ Edge Cases
- Multiple simultaneous connections
- Rapid command execution
- Invalid parameter combinations
- Disconnection handling

## Example Usage

### Running Basic Tests
```bash
# Start your server first
./ft_irc 6667 test123

# In another terminal, run quick test
./quick_test.sh 6667 test123
```

### Running Comprehensive Tests
```bash
# With default settings (port 6667, password test123)
./test_irc.sh

# The script will automatically detect if server is running
```

### Stress Testing
```bash
# Test with 20 concurrent clients
./stress_test.sh 6667 test123 20

# Monitor your server output for performance
```

### Interactive Testing
```bash
# Connect as "MyNick"
./irc_client.sh 6667 test123 MyNick

# Then type IRC commands interactively:
> JOIN #test
> PRIVMSG #test :Hello everyone!
> TOPIC #test :My test channel
> exit
```

## Expected Server Output

During testing, you should see output similar to:
```
Server is running on port 6667
New client connected: fd = 4
IRC message from {4} : [PASS test123]
IRC message from {4} : [NICK TestUser1]
IRC message from {4} : [USER test1 0 * :Test User One]
IRC message from {4} : [JOIN #test]
IRC message from {4} : [PRIVMSG #test :Hello from TestUser1!]
...
```

## Troubleshooting

### "Server not running" error
- Make sure your IRC server is compiled: `make`
- Start the server: `./ft_irc 6667 test123`
- Check if port is already in use: `netstat -tulpn | grep 6667`

### Tests fail immediately
- Verify server accepts connections: `nc -z localhost 6667`
- Check server password matches test scripts
- Ensure no firewall blocking connections

### Permission denied on scripts
- Make scripts executable: `chmod +x *.sh`

### netcat not found
- Install netcat: `sudo apt-get install netcat` (Ubuntu/Debian)
- Or use telnet: `telnet localhost 6667`

## Adding Custom Tests

To add your own test scenarios:

1. **Create a new bot script** following the pattern in `test_irc.sh`
2. **Use the command file approach** for complex sequences
3. **Add appropriate sleep delays** between commands
4. **Include cleanup** in your scripts

Example custom bot:
```bash
cat > /tmp/custom_bot_commands.txt << 'EOF'
JOIN #mychannel
PRIVMSG #mychannel :Custom test message
MODE #mychannel +i
INVITE SomeUser #mychannel
QUIT :Custom test complete
EOF

create_bot "custom_bot" "CustomNick" "custom" "Custom User" "/tmp/custom_bot_commands.txt"
```

## IRC Standards Compliance

These tests verify compliance with basic IRC RFC standards:
- RFC 1459 (Internet Relay Chat Protocol)
- RFC 2810 (Internet Relay Chat: Architecture)
- RFC 2811 (Internet Relay Chat: Channel Management)
- RFC 2812 (Internet Relay Chat: Client Protocol)

Happy testing! 🚀
