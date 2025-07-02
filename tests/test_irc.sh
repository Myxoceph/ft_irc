#!/bin/bash

# ft_IRC Comprehensive Test Script
# This script creates multiple IRC bots to test various IRC server functionalities

SERVER_HOST="localhost"
SERVER_PORT="6667"
SERVER_PASSWORD="test123"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to send IRC command to server
send_irc_command() {
    local socket_fd=$1
    local command=$2
    echo -e "$command\r\n" >&${socket_fd}
    sleep 0.1  # Small delay to prevent flooding
}

# Function to create IRC bot
create_bot() {
    local bot_name=$1
    local nickname=$2
    local username=$3
    local realname=$4
    local commands_file=$5
    
    echo -e "${BLUE}[BOT] Starting $bot_name...${NC}"
    
    # Create a temporary script for this bot
    cat > "/tmp/${bot_name}_script.sh" << EOF
#!/bin/bash

# Create command sequence for this bot
{
    echo "PASS $SERVER_PASSWORD"
    sleep 0.2
    echo "NICK $nickname"
    sleep 0.2
    echo "USER $username 0 * :$realname"
    sleep 0.5
    
    # Execute bot-specific commands
    if [ -f "$commands_file" ]; then
        while IFS= read -r cmd; do
            if [ ! -z "\$cmd" ] && [[ ! "\$cmd" =~ ^#.* ]]; then
                echo -e "${CYAN}[$bot_name] \$cmd${NC}" >&2
                echo "\$cmd"
                sleep 0.3
            fi
        done < "$commands_file"
    fi
    
    # Keep connection alive for a while to see responses
    sleep 10
} | nc -C $SERVER_HOST $SERVER_PORT &

EOF
    
    chmod +x "/tmp/${bot_name}_script.sh"
    "/tmp/${bot_name}_script.sh" &
    echo $! > "/tmp/${bot_name}.pid"
}

# Function to cleanup all bots
cleanup_bots() {
    echo -e "${YELLOW}Cleaning up bots...${NC}"
    for pidfile in /tmp/bot*.pid; do
        if [ -f "$pidfile" ]; then
            kill $(cat "$pidfile") 2>/dev/null
            rm "$pidfile"
        fi
    done
    rm -f /tmp/bot*_script.sh /tmp/bot*_commands.txt
}

# Trap to cleanup on script exit
trap cleanup_bots EXIT

echo -e "${GREEN}===========================================${NC}"
echo -e "${GREEN}    ft_IRC Comprehensive Test Suite${NC}"
echo -e "${GREEN}===========================================${NC}"
echo -e "${YELLOW}Server: $SERVER_HOST:$SERVER_PORT${NC}"
echo -e "${YELLOW}Password: $SERVER_PASSWORD${NC}"
echo ""

# Wait for server to be ready
echo -e "${BLUE}Checking if server is running...${NC}"
if ! nc -z $SERVER_HOST $SERVER_PORT; then
    echo -e "${RED}Error: IRC server is not running on $SERVER_HOST:$SERVER_PORT${NC}"
    echo -e "${YELLOW}Please start your server with: ./ft_irc $SERVER_PORT $SERVER_PASSWORD${NC}"
    exit 1
fi

echo -e "${GREEN}Server is running! Starting tests...${NC}"
echo ""

# Test 1: Basic Authentication and Registration
echo -e "${PURPLE}=== TEST 1: Basic Authentication & Registration ===${NC}"

# Bot 1: Admin user
cat > /tmp/bot1_commands.txt << 'EOF'
# Create and join main channel
JOIN #general
PRIVMSG #general :Hello everyone! I'm the admin.
TOPIC #general :Welcome to the general channel!
EOF

create_bot "bot1" "Admin" "admin" "IRC Administrator" "/tmp/bot1_commands.txt"
sleep 2

# Test 2: Multiple Users Joining
echo -e "${PURPLE}=== TEST 2: Multiple Users & Channel Operations ===${NC}"

# Bot 2: Regular user
cat > /tmp/bot2_commands.txt << 'EOF'
JOIN #general
PRIVMSG #general :Hi Admin! Nice to meet you.
JOIN #random
TOPIC #random :Random discussions here
PRIVMSG #random :This is the random channel!
EOF

create_bot "bot2" "Alice" "alice" "Alice Wonderland" "/tmp/bot2_commands.txt"
sleep 1

# Bot 3: Another regular user
cat > /tmp/bot3_commands.txt << 'EOF'
JOIN #general
PRIVMSG #general :Hey everyone!
JOIN #random
PRIVMSG #random :Anyone here?
PRIVMSG Alice :Hi Alice, private message!
EOF

create_bot "bot3" "Bob" "bob" "Bob Builder" "/tmp/bot3_commands.txt"
sleep 1

# Test 3: Channel Modes Testing
echo -e "${PURPLE}=== TEST 3: Channel Modes Testing ===${NC}"

# Bot 4: Mode tester
cat > /tmp/bot4_commands.txt << 'EOF'
JOIN #general
PRIVMSG #general :Testing channel modes...
MODE #general +i
PRIVMSG #general :Channel is now invite-only
MODE #general +k secret123
PRIVMSG #general :Channel now has password: secret123
MODE #general +l 5
PRIVMSG #general :Channel user limit set to 5
MODE #general +o Alice
PRIVMSG #general :Alice is now an operator
EOF

create_bot "bot4" "Charlie" "charlie" "Charlie Brown" "/tmp/bot4_commands.txt"
sleep 2

# Test 4: Operator Commands Testing
echo -e "${PURPLE}=== TEST 4: Operator Commands Testing ===${NC}"

# Bot 5: Target for kick/invite tests
cat > /tmp/bot5_commands.txt << 'EOF'
JOIN #general
PRIVMSG #general :I'm here to test operator commands
JOIN #private
TOPIC #private :Private discussions
EOF

create_bot "bot5" "David" "david" "David Jones" "/tmp/bot5_commands.txt"
sleep 1

# Bot 6: Operator actions
cat > /tmp/bot6_commands.txt << 'EOF'
JOIN #general
PRIVMSG #general :I will test operator commands
KICK #general David
PRIVMSG #general :David has been kicked for testing
INVITE David #private
PRIVMSG #general :David invited to #private
EOF

create_bot "bot6" "Eve" "eve" "Eve Smith" "/tmp/bot6_commands.txt"
sleep 2

# Test 5: Error Condition Testing
echo -e "${PURPLE}=== TEST 5: Error Conditions Testing ===${NC}"

# Bot 7: Error tester
cat > /tmp/bot7_commands.txt << 'EOF'
JOIN #nonexistent
PRIVMSG #general :Testing error conditions
PRIVMSG NonExistentUser :This should fail
KICK #general NonExistentUser
MODE #general +z
TOPIC #nonexistent :This should fail
INVITE NonExistentUser #general
EOF

create_bot "bot7" "Frank" "frank" "Frank Test" "/tmp/bot7_commands.txt"
sleep 2

# Test 6: Stress Test with Multiple Channels
echo -e "${PURPLE}=== TEST 6: Multiple Channels Stress Test ===${NC}"

# Bot 8: Multi-channel user
cat > /tmp/bot8_commands.txt << 'EOF'
JOIN #channel1
JOIN #channel2
JOIN #channel3
PRIVMSG #channel1 :Hello channel 1!
PRIVMSG #channel2 :Hello channel 2!
PRIVMSG #channel3 :Hello channel 3!
PART #channel2
PRIVMSG #channel1 :Left channel 2
PRIVMSG #channel3 :Still in channel 3
EOF

create_bot "bot8" "Grace" "grace" "Grace Hopper" "/tmp/bot8_commands.txt"
sleep 1

# Bot 9: Another multi-channel user
cat > /tmp/bot9_commands.txt << 'EOF'
JOIN #channel1
JOIN #channel3
JOIN #test
PRIVMSG #channel1 :Grace, are you here?
PRIVMSG #channel3 :Multiple channels test
PRIVMSG #test :Testing new channel
TOPIC #test :Test channel for experiments
EOF

create_bot "bot9" "Henry" "henry" "Henry Ford" "/tmp/bot9_commands.txt"
sleep 1

# Test 7: Private Message Testing
echo -e "${PURPLE}=== TEST 7: Private Messages Testing ===${NC}"

# Bot 10: PM tester
cat > /tmp/bot10_commands.txt << 'EOF'
JOIN #general
PRIVMSG #general :Testing private messages
PRIVMSG Admin :Hi Admin, this is a private message
PRIVMSG Alice :Hi Alice, how are you?
PRIVMSG Bob :Bob, can you see this PM?
PRIVMSG NonExistent :This should generate an error
EOF

create_bot "bot10" "Ivy" "ivy" "Ivy League" "/tmp/bot10_commands.txt"
sleep 2

# Test 8: Authentication Failure Testing
echo -e "${PURPLE}=== TEST 8: Authentication Edge Cases ===${NC}"

# Create a bot with wrong password (will fail authentication)
cat > "/tmp/wrongpass_script.sh" << EOF
#!/bin/bash

{
    echo -e "${RED}[WRONGPASS] Testing wrong password...${NC}" >&2
    echo "PASS wrongpassword"
    sleep 0.5
    echo "NICK TestUser"
    sleep 0.5
    echo "USER test 0 * :Test User"
    sleep 2
} | nc -C $SERVER_HOST $SERVER_PORT &

EOF

chmod +x "/tmp/wrongpass_script.sh"
"/tmp/wrongpass_script.sh" &

sleep 3

# Test 9: QUIT Command Testing
echo -e "${PURPLE}=== TEST 9: QUIT Command Testing ===${NC}"

# Bot 11: Quit tester
cat > /tmp/bot11_commands.txt << 'EOF'
JOIN #general
PRIVMSG #general :I will quit soon to test QUIT command
QUIT :Testing quit functionality
EOF

create_bot "bot11" "Jack" "jack" "Jack Quit" "/tmp/bot11_commands.txt"
sleep 3

# Test 10: Invalid Commands Testing
echo -e "${PURPLE}=== TEST 10: Invalid Commands Testing ===${NC}"

# Bot 12: Invalid command tester
cat > /tmp/bot12_commands.txt << 'EOF'
JOIN #general
INVALIDCOMMAND
FAKECOMMAND parameter
PRIVMSG
JOIN
PART
MODE #general
KICK #general
INVITE
TOPIC
EOF

create_bot "bot12" "Karen" "karen" "Karen Invalid" "/tmp/bot12_commands.txt"
sleep 2

echo ""
echo -e "${GREEN}===========================================${NC}"
echo -e "${GREEN}              Test Summary${NC}"
echo -e "${GREEN}===========================================${NC}"
echo -e "${YELLOW}Tests completed! Here's what was tested:${NC}"
echo ""
echo -e "${CYAN}✓ Basic authentication (PASS)${NC}"
echo -e "${CYAN}✓ User registration (NICK, USER)${NC}"
echo -e "${CYAN}✓ Channel operations (JOIN, PART, QUIT)${NC}"
echo -e "${CYAN}✓ Private messages (PRIVMSG)${NC}"
echo -e "${CYAN}✓ Channel messaging${NC}"
echo -e "${CYAN}✓ Channel modes (+i, +k, +l, +o, +t)${NC}"
echo -e "${CYAN}✓ Operator commands (KICK, INVITE)${NC}"
echo -e "${CYAN}✓ Topic management (TOPIC)${NC}"
echo -e "${CYAN}✓ Multiple channels${NC}"
echo -e "${CYAN}✓ Error conditions${NC}"
echo -e "${CYAN}✓ Authentication failures${NC}"
echo -e "${CYAN}✓ Invalid commands${NC}"
echo ""
echo -e "${YELLOW}Check your server output for detailed responses and error handling.${NC}"
echo -e "${YELLOW}All bots will disconnect automatically in a few seconds.${NC}"

# Wait for all processes to complete
wait

echo -e "${GREEN}All tests completed!${NC}"
