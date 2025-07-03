#!/bin/bash

# Simple IRC Server Test using netcat with -C option
# Quick automated test without manual intervention

SERVER_HOST="localhost"
SERVER_PORT="6667"
SERVER_PASSWORD="test123"
CHANNEL="#test"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${YELLOW}=== Simple IRC Test (netcat -C) ===${NC}"

# Check netcat availability
if ! command -v nc &> /dev/null; then
    echo -e "${RED}Error: netcat (nc) not found${NC}"
    exit 1
fi

# Test server connectivity
echo -e "${YELLOW}Testing server connectivity...${NC}"
if ! nc -z ${SERVER_HOST} ${SERVER_PORT}; then
    echo -e "${RED}Error: Cannot connect to ${SERVER_HOST}:${SERVER_PORT}${NC}"
    echo "Start your server: ./ft_irc ${SERVER_PORT} ${SERVER_PASSWORD}"
    exit 1
fi
echo -e "${GREEN}✓ Server is reachable${NC}"

# Function to create a test bot
create_test_bot() {
    local nick="$1"
    local test_msg="$2"
    
    {
        echo "PASS ${SERVER_PASSWORD}"
        echo "NICK ${nick}"
        echo "USER ${nick,,} 0 * :${nick} Test Bot"
        sleep 1
        echo "JOIN ${CHANNEL}"
        sleep 1
        echo "PRIVMSG ${CHANNEL} :${test_msg}"
        sleep 2
        echo "QUIT :Test completed"
    } | nc -C ${SERVER_HOST} ${SERVER_PORT} &
    
    echo -e "${GREEN}✓ ${nick} started${NC}"
}

# Test 1: Single connection
echo -e "${YELLOW}Test 1: Single bot connection${NC}"
create_test_bot "TestBot1" "Hello from TestBot1!"
sleep 3

# Test 2: Multiple connections
echo -e "${YELLOW}Test 2: Multiple bot connections${NC}"
create_test_bot "Bot2" "Greetings from Bot2!"
create_test_bot "Bot3" "Bot3 reporting for duty!"
create_test_bot "Bot4" "Bot4 is here!"
create_test_bot "Bot5" "Bot5 ready to test!"

sleep 5

# Test 3: Advanced commands
echo -e "${YELLOW}Test 3: Advanced commands${NC}"
{
    echo "PASS ${SERVER_PASSWORD}"
    echo "NICK AdminBot"
    echo "USER admin 0 * :Admin Test Bot"
    sleep 1
    echo "JOIN ${CHANNEL}"
    sleep 1
    echo "PRIVMSG ${CHANNEL} :Testing advanced commands..."
    echo "TOPIC ${CHANNEL} :Test topic from AdminBot"
    echo "PRIVMSG ${CHANNEL} :Topic set!"
    sleep 1
    echo "PRIVMSG Bot2 :Private message test to Bot2"
    echo "PRIVMSG ${CHANNEL} :Private message sent!"
    sleep 1
    echo "PART ${CHANNEL} :Testing PART command"
    sleep 1
    echo "JOIN ${CHANNEL}"
    echo "PRIVMSG ${CHANNEL} :Rejoined successfully!"
    sleep 2
    echo "PRIVMSG ${CHANNEL} :All tests completed! 🎉"
    sleep 1
    echo "QUIT :All tests finished"
} | nc -C ${SERVER_HOST} ${SERVER_PORT} &

echo -e "${GREEN}✓ AdminBot started for advanced tests${NC}"

echo -e "${YELLOW}Waiting for tests to complete...${NC}"
sleep 8

echo -e "${GREEN}=== Simple tests completed! ===${NC}"
echo -e "Check your server output to verify all commands worked correctly."
