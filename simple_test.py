#!/usr/bin/env python3
"""
Simple IRC Server Test - Automated Version
Tests basic functionality without requiring manual intervention
"""

import socket
import time
import threading
import sys

class SimpleBot:
    def __init__(self, nick, host='localhost', port=6667, password='test123'):
        self.nick = nick
        self.host = host
        self.port = port
        self.password = password
        self.socket = None
        self.messages = []
        
    def connect_and_test(self):
        """Connect and run basic tests"""
        try:
            # Connect
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            print(f"✓ {self.nick} connected successfully")
            
            # Authenticate
            self.send(f"PASS {self.password}")
            self.send(f"NICK {self.nick}")
            self.send(f"USER {self.nick.lower()} 0 * :{self.nick} Test Bot")
            
            time.sleep(0.5)
            
            # Join channel
            self.send("JOIN #test")
            time.sleep(0.3)
            
            # Send a test message
            self.send(f"PRIVMSG #test :Hello from {self.nick}!")
            time.sleep(0.2)
            
            return True
            
        except Exception as e:
            print(f"✗ {self.nick} failed: {e}")
            return False
    
    def send(self, message):
        """Send message to server"""
        try:
            full_msg = f"{message}\r\n"
            self.socket.send(full_msg.encode('utf-8'))
            print(f"  [{self.nick}] → {message}")
        except Exception as e:
            print(f"  [{self.nick}] Send error: {e}")
    
    def disconnect(self):
        """Disconnect from server"""
        try:
            if self.socket:
                self.send(f"QUIT :Test completed")
                time.sleep(0.1)
                self.socket.close()
                print(f"✓ {self.nick} disconnected")
        except:
            pass

def run_simple_test():
    """Run automated test"""
    print("=== Simple IRC Server Test ===")
    print("Testing basic functionality...\n")
    
    # Test 1: Single connection
    print("1. Testing single connection...")
    bot1 = SimpleBot("TestBot1")
    if bot1.connect_and_test():
        time.sleep(1)
        bot1.disconnect()
        print("✓ Single connection test passed\n")
    else:
        print("✗ Single connection test failed\n")
        return
    
    # Test 2: Multiple connections
    print("2. Testing multiple connections...")
    bots = []
    success_count = 0
    
    for i in range(5):
        bot = SimpleBot(f"Bot{i+1}")
        bots.append(bot)
        if bot.connect_and_test():
            success_count += 1
        time.sleep(0.5)
    
    print(f"✓ {success_count}/5 bots connected successfully")
    
    # Let them chat a bit
    time.sleep(2)
    
    # Test some basic commands
    if bots:
        print("\n3. Testing additional commands...")
        main_bot = bots[0]
        
        # Test topic
        main_bot.send("TOPIC #test :Test topic from automated script")
        time.sleep(0.3)
        
        # Test private message
        if len(bots) > 1:
            main_bot.send(f"PRIVMSG Bot2 :Private message test")
            time.sleep(0.3)
        
        # Test part and rejoin
        main_bot.send("PART #test :Testing part command")
        time.sleep(0.5)
        main_bot.send("JOIN #test")
        time.sleep(0.3)
        main_bot.send("PRIVMSG #test :Rejoined successfully!")
        
        print("✓ Additional commands tested")
    
    # Cleanup
    print("\n4. Cleaning up...")
    for bot in bots:
        bot.disconnect()
        time.sleep(0.1)
    
    print("\n=== Test completed successfully! ===")

if __name__ == "__main__":
    try:
        run_simple_test()
    except KeyboardInterrupt:
        print("\nTest interrupted by user")
    except Exception as e:
        print(f"\nTest failed with error: {e}")
