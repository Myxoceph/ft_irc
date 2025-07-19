#!/bin/bash

# JoJo's Bizarre Adventure IRC Bot Tester - Stand User behavior
# Usage: ./irc_bot_tester.sh <password>

if [ $# -ne 1 ]; then
    echo "Usage: $0 <server_password>"
    echo "Example: $0 mypassword"
    exit 1
fi

SERVER_PASSWORD="$1"
SERVER="localhost"
PORT="6667"
CHANNEL="#test"

# JoJo Stand Users and behaviors
declare -A NPC_NAMES=([1]="Jotaro" [2]="Dio" [3]="Giorno" [4]="Josuke" [5]="Jonathan")
declare -A NPC_USERNAMES=([1]="star_platinum" [2]="the_world" [3]="gold_experience" [4]="crazy_diamond" [5]="hamon_user")
declare -A NPC_REALNAMES=([1]="Jotaro Kujo" [2]="Dio Brando" [3]="Giorno Giovanna" [4]="Josuke Higashikata" [5]="Jonathan Joestar")
declare -A NPC_NEW_NICKS=([1]="JotaroORA" [2]="DioMUDA" [3]="GiornoGER" [4]="JosukeDORA" [5]="JonathanHamon")

# JoJo Stand User messages with their iconic phrases
declare -A NPC_MESSAGES_1=(
    [1]="Yare yare daze... *tips hat* Jotaro Kujo has entered the channel. 🎩"
    [2]="KONO DIO DA! *dramatic pose* You thought it would be someone else, but it was me, DIO! ⚡"
    [3]="I, Giorno Giovanna, have a dream! And that dream starts with joining this channel! ✨"
    [4]="Oi! Josuke here! What a beautiful Duwang this channel is! Ready to fix anything broken! 💎"
    [5]="A true gentleman always announces his arrival! Jonathan Joestar, at your service! 🌟"
)

declare -A NPC_MESSAGES_2=(
    [1]="JotaroORA here... Star Platinum: THE WORLD! Time has resumed and so have I. ⭐"
    [2]="DioMUDA activated! MUDA MUDA MUDA! Your IRC server cannot escape THE WORLD! �"
    [3]="GiornoGER ready! This is... Requiem. My Gold Experience has evolved beyond limits! 🏆"
    [4]="JosukeDORA reporting! DORA DORA DORA! Everything's getting fixed and improved! ⚡"
    [5]="JonathanHamon charged! The power of Hamon flows through this connection! ☀️"
)

declare -A NPC_PRIVATE_MESSAGES=(
    [1]="abakirca... *adjusts hat* Jotaro here. Your server's performance is... not bad. Good grief. �"
    [2]="abakirca! It was I, DIO, who tested your server! WRYYY! Your code shows potential for immortality! ⚡"
    [3]="abakirca, this is Giorno. I have a dream that your IRC server will become the best! Molto bene! ✨"
    [4]="Yo abakirca! Josuke speaking! Your server's running smoother than my hair! Greato daze! �"
    [5]="Dear abakirca, Jonathan Joestar here! Your noble effort in creating this server warms my heart! 🌟"
)

declare -A NPC_QUIT_MESSAGES=(
    [1]="Yare yare daze... Jotaro's leaving. Time to go save the world again. �"
    [2]="KONO DIO DA! I must depart to conquer more realms! WRYYYY! ⚡"
    [3]="Arrivederci! Giorno's dream continues elsewhere... Ciao! 🌟"
    [4]="Josuke's gotta bounce! Someone's probably insulting my hair somewhere! 💥"
    [5]="Farewell, friends! Jonathan departs with the spirit of a true gentleman! ⚔️"
)

declare -A NPC_PART_MESSAGES=(
    [1]="Star Platinum is calling... Jotaro stepping out for a moment!"
    [2]="THE WORLD demands my attention! Dio taking a strategic pause!"
    [3]="Giorno conducting some Golden Wind business... be right back!"
    [4]="Josuke fixing something urgent... Diamond is Unbreakable!"
    [5]="Jonathan practicing Hamon elsewhere... temporary gentleman's departure!"
)

# Function to create a JoJo Stand User session
create_npc() {
    local npc_id=$1
    local nick="${NPC_NAMES[$npc_id]}"
    local new_nick="${NPC_NEW_NICKS[$npc_id]}"
    local username="${NPC_USERNAMES[$npc_id]}"
    local realname="${NPC_REALNAMES[$npc_id]}"
    local part_before_quit=$2
    
    # Start the NPC behavior simulation
    (
        # Connect to IRC server with humanoid behavior
        # SEQUENCE: PASS → NICK → USER → JOIN #test → PRIVMSG #test → NICK change → PRIVMSG #test → PART/QUIT
        {
            # Initial connection delay (like opening IRC client)
            sleep 3
            
            printf "PASS $SERVER_PASSWORD\r\n"
            sleep 2
            
            printf "NICK $nick\r\n"
            sleep 2
            
            printf "USER $username 0 * :$realname\r\n"
            
            # Wait for registration to complete (human would wait to see welcome message)
            sleep 5
            sleep 1
            
            # Join channel (human would type /join #test)
            sleep 2
            printf "JOIN $CHANNEL\r\n"
            
            # CRITICAL: Wait for JOIN to be processed by server and confirmed
            # Humans would see the join message, topic, and user list before talking
            sleep 5
            sleep 2
            sleep 3
            
            # Now safely in channel - start participating
            
            # STEP 1: First message to channel (after joining)
            local msg1="${NPC_MESSAGES_1[$npc_id]}"
            sleep 2
            printf "PRIVMSG $CHANNEL :$msg1\r\n"
            
            # Wait between messages (humans don't spam)
            sleep 4
            
            # Send private message to abakirca (like humans would notice the admin)
            local priv_msg="${NPC_PRIVATE_MESSAGES[$npc_id]}"
            sleep 2
            printf "PRIVMSG abakirca :$priv_msg\r\n"
            
            # Wait before changing nickname
            sleep 5
            
            # STEP 2: Change nickname
            sleep 3
            printf "NICK $new_nick\r\n"
            
            # Wait for nick change to be processed
            sleep 8
            
            # STEP 3: Second message to channel (after nick change)
            local msg2="${NPC_MESSAGES_2[$npc_id]}"
            sleep 2
            printf "PRIVMSG $CHANNEL :$msg2\r\n"
            
            # Stay a bit longer with new nick (humans don't immediately leave)
            sleep 5
            
            # Decide what to do based on NPC personality
            if [ "$part_before_quit" = "true" ]; then
                sleep 3
                local part_msg="${NPC_PART_MESSAGES[$npc_id]}"
                printf "PART $CHANNEL :$part_msg\r\n"
                
                # Brief pause before completely disconnecting
                sleep 3
                local quit_msg="${NPC_QUIT_MESSAGES[$npc_id]}"
                printf "QUIT :$quit_msg\r\n"
            else
                sleep 3
                local quit_msg="${NPC_QUIT_MESSAGES[$npc_id]}"
                printf "QUIT :$quit_msg\r\n"
            fi
            
            # Ensure final command is sent before pipe closes
            sleep 3
            
        } | nc "$SERVER" "$PORT" > "/tmp/npc${npc_id}_chat.log" 2>&1 &
        
        local nc_pid=$!
        
        # Let the NPC complete its full session (total time is ~65 seconds)
        sleep 80  # Increased timeout to allow all commands to complete
        
        # Cleanup if still connected
        kill $nc_pid 2>/dev/null
        
    ) &
}

# Deploy NPCs to the IRC world

# Deploy NPCs with staggered arrival times (like real users joining)
create_npc 1 true
sleep 5

create_npc 2 true  
sleep 5

create_npc 3 true
sleep 5

create_npc 4 false
sleep 5

create_npc 5 false
sleep 3

# Wait for all NPCs to complete their life cycles
wait
