#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Max buff
#define BUFFER_SIZE 70000

// Handle the error message and exit
void handleError(const char *message) { 
    perror(message); 
    exit(EXIT_FAILURE); 
}

int sendAllData(int socketDescriptor, char *dataBuffer, int dataLength);
int receiveAllData(int socketDescriptor, char *dataBuffer, int dataLength);

int main(int argc, char *argv[]) {
    int sockFD, portNum, sentBytes, receivedBytes;
    struct sockaddr_in serverAddr;
    struct hostent *serverInfo;
    char plainTextBuffer[BUFFER_SIZE];
    char encryptionKeyBuffer[BUFFER_SIZE];
    char encodedMessageBuffer[BUFFER_SIZE];

    // Initialize buffers
    memset(plainTextBuffer, '\0', BUFFER_SIZE);
    memset(encryptionKeyBuffer, '\0', BUFFER_SIZE);
    memset(encodedMessageBuffer, '\0', BUFFER_SIZE);

    // Validate input arguments
    if (argc < 4) {
        fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&serverAddr, '\0', sizeof(serverAddr));
    portNum = atoi(argv[3]);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portNum);
    serverInfo = gethostbyname("localhost");

    if (!serverInfo) {
        fprintf(stderr, "ERROR: No such host found.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(&serverAddr.sin_addr.s_addr, serverInfo->h_addr, serverInfo->h_length);

    // Create socket
    sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0) {
        handleError("ERROR: Unable to open socket.");
    }

    // Connect to server
    if (connect(sockFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        handleError("ERROR: Connection failed.");
    }

    // Read plaintext file
    FILE *plainTextFile = fopen(argv[1], "r");
    if (!plainTextFile) {
        fprintf(stderr, "ERROR: Unable to open plaintext file.\n");
        exit(EXIT_FAILURE);
    }
    fgets(plainTextBuffer, BUFFER_SIZE, plainTextFile);
    fclose(plainTextFile);
    plainTextBuffer[strcspn(plainTextBuffer, "\n")] = '\0';

    // Read key file
    FILE *keyFile = fopen(argv[2], "r");
    if (!keyFile) {
        fprintf(stderr, "ERROR: Unable to open key file.\n");
        exit(EXIT_FAILURE);
    }
    fgets(encryptionKeyBuffer, BUFFER_SIZE, keyFile);
    fclose(keyFile);
    encryptionKeyBuffer[strcspn(encryptionKeyBuffer, "\n")] = '\0';

    // Validate plaintext and key
    char validChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    for (int i = 0; i < strlen(plainTextBuffer); i++) {
        if (!strchr(validChars, plainTextBuffer[i])) {
            fprintf(stderr, "ERROR: Invalid characters in plaintext file.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (strlen(encryptionKeyBuffer) < strlen(plainTextBuffer)) {
        fprintf(stderr, "ERROR: Key is too short.\n");
        exit(EXIT_FAILURE);
    }

    // Send data to server
    int textLength = strlen(plainTextBuffer);
    send(sockFD, &textLength, sizeof(textLength), 0);
    sendAllData(sockFD, plainTextBuffer, textLength);
    sendAllData(sockFD, encryptionKeyBuffer, textLength);

    // Receive encrypted message from server
    int encodedLength;
    recv(sockFD, &encodedLength, sizeof(encodedLength), 0);
    receiveAllData(sockFD, encodedMessageBuffer, encodedLength);

    printf("%s\n", encodedMessageBuffer);

    close(sockFD);
    return 0;
}

int sendAllData(int socketDescriptor, char *dataBuffer, int dataLength) {
    int totalSent = 0;
    int bytesLeft = dataLength;
    int bytesSent;

	// While the sent message is less than data (Or else won't work)
    while (totalSent < dataLength) {
		// Send bytes
        bytesSent = send(socketDescriptor, dataBuffer + totalSent, bytesLeft, 0);
        if (bytesSent == -1) {
            handleError("ERROR: Failed to send data.");
        }
        totalSent += bytesSent;
        bytesLeft -= bytesSent;
    }
    return totalSent;
}

int receiveAllData(int socketDescriptor, char *dataBuffer, int dataLength) {
    int totalReceived = 0;
    int bytesLeft = dataLength;
    int bytesRead;

	// Ensure datalength is greater for recieving data
    while (totalReceived < dataLength) {
        bytesRead = recv(socketDescriptor, dataBuffer + totalReceived, bytesLeft, 0);
        if (bytesRead == -1) {
            handleError("ERROR: Failed to receive data.");
        }
        totalReceived += bytesRead;
        bytesLeft -= bytesRead;
    }
    return totalReceived;
}
