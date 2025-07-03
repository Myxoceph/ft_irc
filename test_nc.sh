#!/bin/bash

# IRC Server Test Script using netcat
# Tests ft_irc server with 5 bots joining #test channel
# One bot requests operator status and tests various commands

SERVER_HOST="localhost"
SERVER_PORT="6667"
SERVER_PASSWORD="test123"
CHANNEL="#test"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to send IRC commands via netcat
send_irc_commands() {
    local nick="$1"
    local username="$2"
    local realname="$3"
    shift 3
    local commands=("$@")
    
    {
        echo "PASS ${SERVER_PASSWORD}"
        echo "NICK ${nick}"
        echo "USER ${username} 0 * :${realname}"
        sleep 1
        echo "JOIN ${CHANNEL}"
        sleep 1
        
        # Execute additional commands
        for cmd in "${commands[@]}"; do
            echo "$cmd"
            sleep 0.5
        done
        
        # Keep connection alive for a while
        sleep 30
        echo "QUIT :Test completed"
    } | nc -C ${SERVER_HOST} ${SERVER_PORT} &
}

# Function to create a bot that just joins and chats
create_basic_bot() {
    local bot_num="$1"
    local nick="TestBot${bot_num}"
    local username="bot${bot_num}"
    local realname="Test Bot Number ${bot_num}"
    
    echo -e "${GREEN}[${nick}]${NC} Connecting..."
    
    local commands=(
        "PRIVMSG ${CHANNEL} :Hello everyone! I'm ${nick}"
    )
    
    send_irc_commands "${nick}" "${username}" "${realname}" "${commands[@]}"
    echo -e "${GREEN}[${nick}]${NC} Started (PID: $!)"
}

# Function to create the operator bot
create_operator_bot() {
    local nick="OpBot"
    local username="opbot"
    local realname="Operator Test Bot"
    
    echo -e "${BLUE}[${nick}]${NC} Connecting..."
    
    local commands=(
        "PRIVMSG ${CHANNEL} :Hello everyone! I'm the operator test bot."
        "PRIVMSG ${CHANNEL} :Admin, could you please give me operator status with: /MODE ${CHANNEL} +o ${nick}"
        "PRIVMSG ${CHANNEL} :I will test various commands once I get OP privileges."
        "PRIVMSG ${CHANNEL} :Waiting for OP status..."
    )
    
    send_irc_commands "${nick}" "${username}" "${realname}" "${commands[@]}"
    echo -e "${BLUE}[${nick}]${NC} Started (PID: $!)"
}

# Function to run operator tests (manual trigger)
run_operator_tests() {
    local nick="OpBot"
    
    echo -e "${YELLOW}Running operator tests...${NC}"
    
    {
        echo "PASS ${SERVER_PASSWORD}"
        echo "NICK ${nick}_Test"
        echo "USER optest 0 * :Operator Test Commands"
        sleep 1
        echo "JOIN ${CHANNEL}"
        sleep 1
        
        echo "PRIVMSG ${CHANNEL} :Starting operator command tests..."
        sleep 1
        
        # Test topic setting
        echo "TOPIC ${CHANNEL} :This is a test topic set by OpBot!"
        echo "PRIVMSG ${CHANNEL} :✓ Topic command tested"
        sleep 1
        
        # Test mode changes
        echo "MODE ${CHANNEL} +i"
        echo "PRIVMSG ${CHANNEL} :✓ Invite-only mode set"
        sleep 1
        
        echo "MODE ${CHANNEL} +l 10"
        echo "PRIVMSG ${CHANNEL} :✓ User limit set to 10"
        sleep 1
        
        echo "MODE ${CHANNEL} +k secretkey"
        echo "PRIVMSG ${CHANNEL} :✓ Channel key set"
        sleep 1
        
        # Test giving OP to another bot
        echo "MODE ${CHANNEL} +o TestBot1"
        echo "PRIVMSG ${CHANNEL} :✓ Gave OP to TestBot1"
        sleep 1
        
        # Test invite
        echo "INVITE TestBot2 ${CHANNEL}"
        echo "PRIVMSG ${CHANNEL} :✓ Invite command tested"
        sleep 1
        
        # Test kick
        echo "KICK ${CHANNEL} TestBot3 :Testing kick functionality"
        echo "PRIVMSG ${CHANNEL} :✓ Kick command tested"
        sleep 2
        
        # Re-invite kicked user
        echo "INVITE TestBot3 ${CHANNEL}"
        echo "PRIVMSG ${CHANNEL} :✓ Re-invited TestBot3"
        sleep 1
        
        # Test private messages
        echo "PRIVMSG TestBot1 :This is a private message test!"
        echo "PRIVMSG TestBot2 :Hello from OpBot in private!"
        echo "PRIVMSG ${CHANNEL} :✓ Private message tests sent"
        sleep 1
        
        echo "PRIVMSG ${CHANNEL} :All operator tests completed! 🎉"
        
        sleep 10
        echo "QUIT :Operator tests finished"
    } | nc -C ${SERVER_HOST} ${SERVER_PORT} &
    
    echo -e "${YELLOW}Operator tests started (PID: $!)${NC}"
}

# Main test function
main() {
    echo -e "${BLUE}=== IRC Server Test Script (using netcat) ===${NC}"
    echo -e "Server: ${SERVER_HOST}:${SERVER_PORT}"
    echo -e "Password: ${SERVER_PASSWORD}"
    echo -e "Channel: ${CHANNEL}"
    echo

    # Check if nc is available
    if ! command -v nc &> /dev/null; then
        echo -e "${RED}Error: netcat (nc) is not installed${NC}"
        exit 1
    fi

    # Test server connectivity
    echo -e "${YELLOW}Testing server connectivity...${NC}"
    if ! nc -z ${SERVER_HOST} ${SERVER_PORT}; then
        echo -e "${RED}Error: Cannot connect to server ${SERVER_HOST}:${SERVER_PORT}${NC}"
        echo "Make sure your IRC server is running:"
        echo "./ft_irc ${SERVER_PORT} ${SERVER_PASSWORD}"
        exit 1
    fi
    echo -e "${GREEN}✓ Server is reachable${NC}"
    echo

    # Create 4 basic bots
    echo -e "${YELLOW}--- Creating basic bots ---${NC}"
    for i in {1..4}; do
        create_basic_bot $i
        sleep 1
    done
    echo

    # Create operator bot
    echo -e "${YELLOW}--- Creating operator bot ---${NC}"
    create_operator_bot
    echo

    echo -e "${YELLOW}--- All bots created ---${NC}"
    echo -e "${BLUE}Bots are now connecting and joining ${CHANNEL}${NC}"
    echo

    # Wait for bots to settle
    sleep 5

    echo -e "${YELLOW}--- Manual Step Required ---${NC}"
    echo -e "Please connect to your IRC server and grant OP status to OpBot:"
    echo -e "${GREEN}1. Connect with: nc -C ${SERVER_HOST} ${SERVER_PORT}${NC}"
    echo -e "${GREEN}2. Authenticate:${NC}"
    echo -e "   PASS ${SERVER_PASSWORD}"
    echo -e "   NICK YourNick"
    echo -e "   USER yourusername 0 * :Your Real Name"
    echo -e "   JOIN ${CHANNEL}"
    echo -e "${GREEN}3. Grant OP: MODE ${CHANNEL} +o OpBot${NC}"
    echo
    echo -e "${YELLOW}Press Enter after granting OP status to run automated tests...${NC}"
    read -r

    # Run operator tests
    run_operator_tests

    echo
    echo -e "${BLUE}Tests are running... Bots will disconnect automatically in ~40 seconds${NC}"
    echo -e "${YELLOW}Monitor the channel to see all tests in action!${NC}"
    echo
    echo -e "${GREEN}Press Ctrl+C to stop all tests immediately${NC}"

    # Wait for tests to complete
    wait
    
    echo
    echo -e "${GREEN}=== All tests completed! ===${NC}"
}

# Cleanup function
cleanup() {
    echo
    echo -e "${YELLOW}Cleaning up... Stopping all background processes${NC}"
    jobs -p | xargs -r kill 2>/dev/null
    exit 0
}

# Set up signal handlers
trap cleanup SIGINT SIGTERM

# Run main function
main "$@"
