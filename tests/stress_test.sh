#!/bin/bash

# IRC Stress Test Script
# Tests server performance with multiple concurrent connections

SERVER_HOST="localhost"
SERVER_PORT=${1:-6667}
SERVER_PASSWORD=${2:-test123}
NUM_CLIENTS=${3:-10}

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${GREEN}IRC Stress Test${NC}"
echo -e "${YELLOW}Creating $NUM_CLIENTS concurrent connections...${NC}"

# Check if server is running
if ! nc -z $SERVER_HOST $SERVER_PORT; then
    echo -e "${RED}Error: IRC server is not running on $SERVER_HOST:$SERVER_PORT${NC}"
    exit 1
fi

# Function to create stress test client
create_stress_client() {
    local client_id=$1
    local client_script="/tmp/stress_client_${client_id}.sh"
    
    cat > "$client_script" << EOF
#!/bin/bash

{
    # Quick authentication
    echo "PASS $SERVER_PASSWORD"
    echo "NICK StressUser${client_id}"
    echo "USER stress${client_id} 0 * :Stress Test User ${client_id}"
    sleep 0.1
    
    # Join multiple channels and send messages
    for i in {1..5}; do
        echo "JOIN #stress\${i}"
        sleep 0.05
        echo "PRIVMSG #stress\${i} :Message from StressUser${client_id}"
        sleep 0.05
    done
    
    # Send some private messages
    for target in {1..3}; do
        if [ \$target -ne $client_id ]; then
            echo "PRIVMSG StressUser\${target} :PM from StressUser${client_id}"
            sleep 0.05
        fi
    done
    
    # Rapid-fire messages
    for i in {1..10}; do
        echo "PRIVMSG #stress1 :Rapid message \${i} from StressUser${client_id}"
        sleep 0.02
    done
    
    # Hold connection for a bit
    sleep 2
    
    # Cleanup
    echo "QUIT :Stress test complete"
} | nc -C $SERVER_HOST $SERVER_PORT &

EOF

    chmod +x "$client_script"
    "$client_script" &
    echo $! > "/tmp/stress_client_${client_id}.pid"
}

# Cleanup function
cleanup_stress_clients() {
    echo -e "${YELLOW}Cleaning up stress test clients...${NC}"
    for i in $(seq 1 $NUM_CLIENTS); do
        if [ -f "/tmp/stress_client_${i}.pid" ]; then
            kill $(cat "/tmp/stress_client_${i}.pid") 2>/dev/null
            rm -f "/tmp/stress_client_${i}.pid" "/tmp/stress_client_${i}.sh"
        fi
    done
}

trap cleanup_stress_clients EXIT

# Create all clients rapidly
echo -e "${BLUE}Starting stress test with $NUM_CLIENTS clients...${NC}"
for i in $(seq 1 $NUM_CLIENTS); do
    create_stress_client $i
    sleep 0.1  # Small delay between client creations
done

echo -e "${YELLOW}All clients started. Monitoring...${NC}"

# Wait for all clients to finish
wait

echo -e "${GREEN}Stress test completed!${NC}"
echo -e "${YELLOW}Check your server logs for performance metrics.${NC}"
