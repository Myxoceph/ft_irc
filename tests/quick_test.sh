#!/bin/bash

# Quick IRC Test Script - Basic functionality test
# Usage: ./quick_test.sh [port] [password]

SERVER_HOST="localhost"
SERVER_PORT=${1:-6667}
SERVER_PASSWORD=${2:-test123}

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}Quick IRC Test for ft_irc${NC}"
echo -e "${YELLOW}Server: $SERVER_HOST:$SERVER_PORT${NC}"
echo -e "${YELLOW}Password: $SERVER_PASSWORD${NC}"
echo ""

# Check if server is running
if ! nc -z $SERVER_HOST $SERVER_PORT; then
    echo -e "${RED}Error: IRC server is not running on $SERVER_HOST:$SERVER_PORT${NC}"
    echo -e "${YELLOW}Please start your server with: ./ft_irc $SERVER_PORT $SERVER_PASSWORD${NC}"
    exit 1
fi

echo -e "${GREEN}Server is running! Creating test clients...${NC}"

# Create first test client
cat > /tmp/test_client1.sh << EOF
#!/bin/bash

{
    echo -e "${BLUE}[CLIENT1] Connecting...${NC}" >&2
    
    # Authentication and registration
    echo "PASS $SERVER_PASSWORD"
    sleep 0.3
    echo "NICK TestUser1"
    sleep 0.3
    echo "USER test1 0 * :Test User One"
    sleep 0.5
    
    # Join channel and send messages
    echo "JOIN #test"
    sleep 0.3
    echo "PRIVMSG #test :Hello from TestUser1!"
    sleep 0.3
    echo "TOPIC #test :Test Channel Topic"
    sleep 2
} | nc -C $SERVER_HOST $SERVER_PORT &

EOF

# Create second test client
cat > /tmp/test_client2.sh << EOF
#!/bin/bash

{
    echo -e "${CYAN}[CLIENT2] Connecting...${NC}" >&2
    
    # Authentication and registration
    echo "PASS $SERVER_PASSWORD"
    sleep 0.3
    echo "NICK TestUser2"
    sleep 0.3
    echo "USER test2 0 * :Test User Two"
    sleep 0.5
    
    # Join channel and send messages
    echo "JOIN #test"
    sleep 0.3
    echo "PRIVMSG #test :Hello from TestUser2!"
    sleep 0.3
    echo "PRIVMSG TestUser1 :Private message to TestUser1"
    sleep 2
} | nc -C $SERVER_HOST $SERVER_PORT &

EOF

chmod +x /tmp/test_client1.sh /tmp/test_client2.sh

# Run clients
echo -e "${YELLOW}Starting test clients...${NC}"
/tmp/test_client1.sh &
PID1=$!
sleep 1
/tmp/test_client2.sh &
PID2=$!

# Wait for clients to finish
wait $PID1
wait $PID2

echo -e "${GREEN}Quick test completed!${NC}"
echo -e "${YELLOW}Check your server output for the IRC messages.${NC}"

# Cleanup
rm -f /tmp/test_client1.sh /tmp/test_client2.sh
