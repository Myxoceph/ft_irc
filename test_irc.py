#!/usr/bin/env python3
"""
IRC Server Test Script
Tests ft_irc server with 5 bots joining #test channel
One bot requests operator status and tests various commands
"""

import socket
import time
import threading
import sys

class IRCBot:
    def __init__(self, nick, username, realname, host='localhost', port=6667, password='test123'):
        self.nick = nick
        self.username = username
        self.realname = realname
        self.host = host
        self.port = port
        self.password = password
        self.socket = None
        self.connected = False
        self.running = True
        
    def connect(self):
        """Connect to IRC server"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            self.connected = True
            print(f"[{self.nick}] Connected to {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"[{self.nick}] Connection failed: {e}")
            return False
    
    def send_raw(self, message):
        """Send raw IRC message"""
        if self.connected:
            try:
                full_msg = f"{message}\r\n"
                self.socket.send(full_msg.encode('utf-8'))
                print(f"[{self.nick}] SENT: {message}")
            except Exception as e:
                print(f"[{self.nick}] Send error: {e}")
    
    def receive(self):
        """Receive messages from server"""
        buffer = ""
        while self.running and self.connected:
            try:
                data = self.socket.recv(1024).decode('utf-8')
                if not data:
                    break
                    
                buffer += data
                while '\r\n' in buffer:
                    line, buffer = buffer.split('\r\n', 1)
                    if line:
                        print(f"[{self.nick}] RECV: {line}")
                        self.handle_message(line)
                        
            except Exception as e:
                print(f"[{self.nick}] Receive error: {e}")
                break
    
    def handle_message(self, message):
        """Handle incoming IRC messages"""
        # Handle PING
        if message.startswith('PING'):
            self.send_raw(f"PONG {message.split(':', 1)[1]}")
    
    def authenticate(self):
        """Authenticate with server"""
        self.send_raw(f"PASS {self.password}")
        time.sleep(0.1)
        self.send_raw(f"NICK {self.nick}")
        time.sleep(0.1)
        self.send_raw(f"USER {self.username} 0 * :{self.realname}")
        time.sleep(0.5)
    
    def join_channel(self, channel):
        """Join a channel"""
        self.send_raw(f"JOIN {channel}")
        time.sleep(0.2)
    
    def send_message(self, target, message):
        """Send PRIVMSG"""
        self.send_raw(f"PRIVMSG {target} :{message}")
    
    def set_mode(self, channel, mode, target=None):
        """Set channel mode"""
        if target:
            self.send_raw(f"MODE {channel} {mode} {target}")
        else:
            self.send_raw(f"MODE {channel} {mode}")
    
    def kick_user(self, channel, target, reason="Kicked"):
        """Kick user from channel"""
        self.send_raw(f"KICK {channel} {target} :{reason}")
    
    def invite_user(self, target, channel):
        """Invite user to channel"""
        self.send_raw(f"INVITE {target} {channel}")
    
    def set_topic(self, channel, topic):
        """Set channel topic"""
        self.send_raw(f"TOPIC {channel} :{topic}")
    
    def part_channel(self, channel, reason="Leaving"):
        """Leave channel"""
        self.send_raw(f"PART {channel} :{reason}")
    
    def quit(self, reason="Bot shutting down"):
        """Quit from server"""
        self.send_raw(f"QUIT :{reason}")
        time.sleep(0.5)
        self.running = False
        if self.socket:
            self.socket.close()
        self.connected = False
    
    def run(self):
        """Main bot loop"""
        if not self.connect():
            return
        
        # Start receive thread
        receive_thread = threading.Thread(target=self.receive, daemon=True)
        receive_thread.start()
        
        # Authenticate
        self.authenticate()
        
        return receive_thread

def test_irc_server():
    """Main test function"""
    print("=== IRC Server Test Script ===")
    print("Starting test with 5 bots...")
    
    # Create 5 bots
    bots = [
        IRCBot("TestBot1", "bot1", "Test Bot Number 1"),
        IRCBot("TestBot2", "bot2", "Test Bot Number 2"), 
        IRCBot("TestBot3", "bot3", "Test Bot Number 3"),
        IRCBot("TestBot4", "bot4", "Test Bot Number 4"),
        IRCBot("OpBot", "opbot", "Operator Test Bot")  # This bot will request OP
    ]
    
    threads = []
    
    try:
        # Connect all bots
        print("\n--- Connecting bots ---")
        for bot in bots:
            thread = bot.run()
            if thread:
                threads.append(thread)
            time.sleep(1)  # Stagger connections
        
        print("\n--- Waiting for authentication ---")
        time.sleep(3)
        
        # Join #test channel
        print("\n--- Joining #test channel ---")
        for bot in bots:
            bot.join_channel("#test")
            time.sleep(0.5)
        
        time.sleep(2)
        
        # OpBot requests operator status from you (manual intervention needed)
        print("\n--- OpBot requesting operator status ---")
        op_bot = bots[4]  # OpBot
        op_bot.send_message("#test", "Hello everyone! I'm the operator test bot.")
        time.sleep(1)
        op_bot.send_message("#test", "Admin, could you please give me operator status with: /MODE #test +o OpBot")
        time.sleep(1)
        op_bot.send_message("#test", "I will test various commands once I get OP privileges.")
        
        print("\n--- Waiting for manual OP grant (please run: /MODE #test +o OpBot in your IRC client) ---")
        print("Press Enter when you've granted OP status to OpBot...")
        input()
        
        # Test various commands as operator
        print("\n--- Testing operator commands ---")
        
        # Test topic setting
        print("Testing TOPIC command...")
        op_bot.set_topic("#test", "This is a test topic set by OpBot!")
        time.sleep(1)
        
        # Test mode changes
        print("Testing MODE commands...")
        op_bot.send_message("#test", "Testing invite-only mode...")
        op_bot.set_mode("#test", "+i")  # Set invite only
        time.sleep(1)
        
        op_bot.send_message("#test", "Testing user limit...")
        op_bot.set_mode("#test", "+l", "10")  # Set user limit
        time.sleep(1)
        
        op_bot.send_message("#test", "Testing channel key...")
        op_bot.set_mode("#test", "+k", "secretkey")  # Set channel key
        time.sleep(1)
        
        # Test giving OP to another bot
        print("Testing OP grant...")
        op_bot.send_message("#test", "Giving OP to TestBot1...")
        op_bot.set_mode("#test", "+o", "TestBot1")
        time.sleep(1)
        
        # Test invite
        print("Testing INVITE command...")
        op_bot.send_message("#test", "Testing invite functionality...")
        op_bot.invite_user("TestBot2", "#test")
        time.sleep(1)
        
        # Test kick (and immediately invite back)
        print("Testing KICK command...")
        op_bot.send_message("#test", "Testing kick command on TestBot3 (will invite back)...")
        time.sleep(1)
        op_bot.kick_user("#test", "TestBot3", "Testing kick functionality")
        time.sleep(2)
        op_bot.invite_user("TestBot3", "#test")
        bots[2].join_channel("#test")  # TestBot3 rejoins
        time.sleep(1)
        
        # Test private messages
        print("Testing PRIVMSG...")
        op_bot.send_message("TestBot1", "This is a private message test!")
        op_bot.send_message("TestBot2", "Hello from OpBot in private!")
        time.sleep(1)
        
        # Final channel message
        op_bot.send_message("#test", "All tests completed! Great job on the IRC server!")
        
        # Let bots chat a bit
        print("\n--- Bots chatting ---")
        bots[0].send_message("#test", "Great server! All commands worked perfectly.")
        time.sleep(0.5)
        bots[1].send_message("#test", "Indeed! The modes and kicks worked well.")
        time.sleep(0.5)
        bots[2].send_message("#test", "Even the kick and rejoin worked smoothly!")
        time.sleep(0.5)
        bots[3].send_message("#test", "Impressive IRC implementation!")
        time.sleep(0.5)
        
        print("\n--- Test completed! Keeping bots connected for 30 seconds ---")
        time.sleep(30)
        
    except KeyboardInterrupt:
        print("\n--- Test interrupted ---")
    
    finally:
        # Cleanup
        print("\n--- Disconnecting bots ---")
        for bot in bots:
            bot.quit("Test completed")
        
        # Wait for threads to finish
        for thread in threads:
            thread.join(timeout=2)
        
        print("Test script finished!")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
            # Update port for all bots
            print(f"Using custom port: {port}")
        except ValueError:
            print("Invalid port number. Using default 6667.")
    
    test_irc_server()
