#!/bin/bash

# IRC Server Test Runner
# Quick script to test your ft_irc server

echo "=== IRC Server Test Runner ==="
echo

# Check if server binary exists
if [ ! -f "./ft_irc" ]; then
    echo "Error: ft_irc binary not found. Please compile first with 'make'"
    exit 1
fi

# Check if Python3 is available
if ! command -v python3 &> /dev/null; then
    echo "Error: Python3 not found. Please install Python3"
    exit 1
fi

echo "Choose test type:"
echo "1. Simple automated test (no manual intervention)"
echo "2. Full test with 5 bots and operator testing (requires manual OP grant)"
echo "3. Start server only (for manual testing)"
echo

read -p "Enter choice (1-3): " choice

case $choice in
    1)
        echo "Starting simple automated test..."
        echo "Make sure to start your server first with: ./ft_irc 6667 test123"
        echo "Press Enter when server is running..."
        read
        python3 simple_test.py
        ;;
    2)
        echo "Starting full test with 5 bots..."
        echo "Make sure to start your server first with: ./ft_irc 6667 test123"
        echo "You will need to manually grant OP status when requested"
        echo "Press Enter when server is running..."
        read
        python3 test_irc.py
        ;;
    3)
        echo "Starting IRC server on port 6667 with password 'test123'..."
        ./ft_irc 6667 test123
        ;;
    *)
        echo "Invalid choice. Exiting."
        exit 1
        ;;
esac
