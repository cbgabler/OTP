#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

// Max dayta for clusters
#define MAX_DATA_SIZE 100000
#define MAX_CLIENTS 5

// Handle error message and terminate program
void handleError(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

// Function declarations for handling client requests and decryption
void handleDecryptionRequest(int clientSocket);
void decryptMessage(const char *ciphertext, const char *key, char *plaintext, int length);

int main(int argc, char *argv[]) {
    int serverSocket, clientSocket, port;
    socklen_t clientAddrLen;
    struct sockaddr_in serverAddr, clientAddr;

    // Ensure port number is provided as argument
    if (argc < 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Convert port argument to integer
    port = atoi(argv[1]);

    // Create server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        handleError("Error creating server socket.");
    }

    // Initialize server address
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket to specified port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        handleError("Error binding server socket.");
    }

    // Listen for incoming client connections
    listen(serverSocket, MAX_CLIENTS);

    // Accept and handle client connections in a loop
    while (1) {
        clientAddrLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            handleError("Error accepting client connection.");
        }

        // Create child process to handle client request
        pid_t childPID = fork();
        if (childPID < 0) {
            handleError("Error creating child process.");
        }

        if (childPID == 0) {
            // Child process: handle decryption request
            close(serverSocket);
            handleDecryptionRequest(clientSocket);
            close(clientSocket);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process: close client socket
            close(clientSocket);
        }
    }

    // Close server socket before exiting
    close(serverSocket);
    return EXIT_SUCCESS;
}

// Process decryption request from client
void handleDecryptionRequest(int clientSocket) {
    char ciphertext[MAX_DATA_SIZE];
    char key[MAX_DATA_SIZE];
    char plaintext[MAX_DATA_SIZE];
    int dataLength;

    // Initialize buffers
    memset(ciphertext, '\0', MAX_DATA_SIZE);
    memset(key, '\0', MAX_DATA_SIZE);
    memset(plaintext, '\0', MAX_DATA_SIZE);

    // Receive data length, ciphertext, and key from client
    recv(clientSocket, &dataLength, sizeof(dataLength), 0);
    recv(clientSocket, ciphertext, dataLength, 0);
    recv(clientSocket, key, dataLength, 0);

    // Perform decryption
    decryptMessage(ciphertext, key, plaintext, dataLength);

    // Send decrypted plaintext back to client
    int plainLength = strlen(plaintext);
    send(clientSocket, &plainLength, sizeof(plainLength), 0);
    send(clientSocket, plaintext, plainLength, 0);
}

// Decrypt message using key and store result in plaintext buffer
void decryptMessage(const char *ciphertext, const char *key, char *plaintext, int length) {
    for (int i = 0; i < length; i++) {
        // Convert characters to integers for modulo operation
        int cipherChar = (ciphertext[i] == ' ') ? 26 : ciphertext[i] - 'A';
        int keyChar = (key[i] == ' ') ? 26 : key[i] - 'A';
        int plainChar = (cipherChar - keyChar + 27) % 27;
        plaintext[i] = (plainChar == 26) ? ' ' : plainChar + 'A';
    }
    plaintext[length] = '\0';
}
