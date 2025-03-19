#!/bin/bash

# Compile server and client programs for encryption and decryption
gcc -o enc_server enc_server.c
gcc -o enc_client enc_client.c
gcc -o dec_server dec_server.c
gcc -o dec_client dec_client.c

# Compile key generation program
gcc -o keygen keygen.c

# Check if any compilation failed
if [ $? -eq 0 ]; then
    echo "All programs compiled successfully!"
else
    echo "Compilation failed."
fi
