#!/bin/bash

# Simple IRC Client for Testing
# Usage: ./irc_client.sh [port] [password] [nickname]

SERVER_HOST="localhost"
SERVER_PORT=${1:-6667}
SERVER_PASSWORD=${2:-test123}
NICKNAME=${3:-TestUser}

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Simple IRC Client${NC}"
echo -e "${YELLOW}Connecting to $SERVER_HOST:$SERVER_PORT as $NICKNAME${NC}"
echo -e "${BLUE}Commands you can use:${NC}"
echo -e "  JOIN #channel"
echo -e "  PRIVMSG #channel :message"
echo -e "  PRIVMSG nickname :private message"
echo -e "  PART #channel"
echo -e "  TOPIC #channel :new topic"
echo -e "  MODE #channel +i (invite only)"
echo -e "  MODE #channel +k password"
echo -e "  MODE #channel +l 10"
echo -e "  MODE #channel +o nickname"
echo -e "  KICK #channel nickname"
echo -e "  INVITE nickname #channel"
echo -e "  QUIT :goodbye message"
echo -e "${YELLOW}Type 'exit' to quit${NC}"
echo ""

# Check if server is running
if ! nc -z $SERVER_HOST $SERVER_PORT; then
    echo -e "${RED}Error: IRC server is not running on $SERVER_HOST:$SERVER_PORT${NC}"
    exit 1
fi

# Create temporary files for communication
FIFO_IN="/tmp/irc_client_in_$$"
FIFO_OUT="/tmp/irc_client_out_$$"
mkfifo "$FIFO_IN" "$FIFO_OUT"

# Cleanup function
cleanup() {
    rm -f "$FIFO_IN" "$FIFO_OUT"
    exit 0
}
trap cleanup EXIT INT TERM

# Connect to server and handle authentication
{
    echo "PASS $SERVER_PASSWORD"
    echo "NICK $NICKNAME"
    echo "USER $NICKNAME 0 * :Test User"
    cat "$FIFO_IN"
} | nc -C $SERVER_HOST $SERVER_PORT | tee "$FIFO_OUT" &
SERVER_PID=$!

# Display server responses in background
{
    while IFS= read -r line; do
        echo -e "${BLUE}[SERVER]${NC} $line"
    done
} < "$FIFO_OUT" &
READER_PID=$!

echo -e "${GREEN}Connected! You can now type IRC commands:${NC}"

# Read user input and send to server
while true; do
    read -p "> " user_input
    
    if [ "$user_input" = "exit" ]; then
        echo "QUIT :Client disconnecting" > "$FIFO_IN"
        break
    elif [ ! -z "$user_input" ]; then
        echo "$user_input" > "$FIFO_IN"
    fi
done

# Cleanup
kill $SERVER_PID $READER_PID 2>/dev/null
cleanup
