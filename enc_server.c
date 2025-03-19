#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define MAX_BUFFER 100000
#define MAX_CONNECTIONS 5

void logError(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void processEncryptionRequest(int connectionFD);
void encryptMessage(const char *plaintext, const char *key, char *ciphertext, int length);

int main(int argc, char *argv[]) {
    int serverSocket, clientSocket, portNumber;
    socklen_t clientAddressSize;
    struct sockaddr_in serverAddress, clientAddress;

	// If argc is too small
    if (argc < 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	// If cannot connect to server
    portNumber = atoi(argv[1]);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        logError("Error creating server socket.");
    }

    memset(&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        logError("Error binding server socket.");
    }

    listen(serverSocket, MAX_CONNECTIONS);

	// While loop to run the server and process the socket request
    while (1) {
        clientAddressSize = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressSize);
        if (clientSocket < 0) {
            logError("Error accepting client connection.");
        }

		// Check if the process can be forked 
        pid_t childPID = fork();
        if (childPID < 0) {
            logError("Error forking child process.");
        }

        if (childPID == 0) {
            close(serverSocket);
            processEncryptionRequest(clientSocket);
            close(clientSocket);
            exit(EXIT_SUCCESS);
        } else {
            close(clientSocket);
        }
    }

	// Close server and end process
    close(serverSocket);
    return 0;
}

void processEncryptionRequest(int connectionFD) {
    // Process encryption
    char plaintext[MAX_BUFFER];
    char key[MAX_BUFFER];
    char ciphertext[MAX_BUFFER];
    int textLength;

    memset(plaintext, '\0', MAX_BUFFER);
    memset(key, '\0', MAX_BUFFER);
    memset(ciphertext, '\0', MAX_BUFFER);


	// Recieve the info
    recv(connectionFD, &textLength, sizeof(textLength), 0);
    recv(connectionFD, plaintext, textLength, 0);
    recv(connectionFD, key, textLength, 0);

    encryptMessage(plaintext, key, ciphertext, textLength);

	// Send the cipher back
    int cipherLength = strlen(ciphertext);
    send(connectionFD, &cipherLength, sizeof(cipherLength), 0);
    send(connectionFD, ciphertext, cipherLength, 0);
}

// Encrypt the message 
void encryptMessage(const char *plaintext, const char *key, char *ciphertext, int length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int i, charIndexPlain, charIndexKey;

    for (i = 0; i < length; i++) {
        charIndexPlain = strchr(charset, plaintext[i]) - charset;
        charIndexKey = strchr(charset, key[i]) - charset;
        ciphertext[i] = charset[(charIndexPlain + charIndexKey) % 27];
    }
}
