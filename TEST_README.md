# IRC Server Test Script

This test script creates 5 bots that will test your ft_irc server functionality.

## How to use:

1. **Start your IRC server:**
   ```bash
   ./ft_irc 6667 test123
   ```

2. **In another terminal, run the test script:**
   ```bash
   python3 test_irc.py
   ```

3. **Connect with your IRC client (like irssi, hexchat, or nc):**
   ```bash
   # Using netcat for simple testing
   nc localhost 6667
   ```
   Then authenticate:
   ```
   PASS test123
   NICK YourNick
   USER yourusername 0 * :Your Real Name
   JOIN #test
   ```

4. **When OpBot asks for operator status, grant it:**
   ```
   MODE #test +o OpBot
   ```

5. **Watch the automated tests run!**

## What the script tests:

- **Connection and Authentication:** All 5 bots connect and authenticate
- **Channel Joining:** All bots join #test channel
- **Operator Request:** OpBot requests OP status from you
- **MODE Commands:** 
  - `+i` (invite only)
  - `+l` (user limit)
  - `+k` (channel key)
  - `+o` (give operator status)
- **TOPIC Command:** Setting channel topic
- **KICK Command:** Kicking and re-inviting users
- **INVITE Command:** Inviting users to channel
- **PRIVMSG:** Both channel and private messages

## Bot Names:
- TestBot1
- TestBot2  
- TestBot3
- TestBot4
- OpBot (the one that requests operator status)

## Notes:
- Default server: localhost:6667
- Default password: test123
- The script will wait for you to manually grant OP to OpBot
- All bots will stay connected for 30 seconds after tests complete
- Press Ctrl+C to stop the test early

## Troubleshooting:
- Make sure your IRC server is running before starting the test
- Ensure port 6667 is available
- Check that Python 3 is installed
- Verify the server password matches "test123"
