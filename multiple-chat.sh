#!/bin/bash

echo "
  __  __      _ _   _      _        ___ _         _   
 |  \/  |_  _| | |_(_)_ __| |___   / __| |_  __ _| |_ 
 | |\/| | || | |  _| | '_ \ / -_) | (__| ' \/ _\` |  _|
 |_|  |_|\_,_|_|\__|_| .__/_\___|  \___|_||_\__,_|\__|
                     |_|

"

# Compile software:
gcc -o chat multiple_chat.c chat.c -lpthread -lncurses -lm

# Execute software:
if [[ "$1" == "-c" ]]; then
    ./chat -c "$2" "$3" "$4"
elif [[ "$1" == "-s" ]]; then
    ./chat -s
else
    echo "ERROR: You have to check the command syntax!"
fi

echo "by jim_bug :)"
