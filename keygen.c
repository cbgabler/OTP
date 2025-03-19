#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CHAR_POOL "ABCDEFGHIJKLMNOPQRSTUVWXYZ "

void generateRandomKey(int length);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s key_length\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int keyLength = atoi(argv[1]);
    if (keyLength <= 0) {
        fprintf(stderr, "Error: Key length must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    generateRandomKey(keyLength);
    return EXIT_SUCCESS;
}

void generateRandomKey(int length) {
    srand((unsigned int)time(NULL));

    for (int i = 0; i < length; i++) {
        printf("%c", CHAR_POOL[rand() % 27]);
    }

    printf("\n"); // Ensure newline at the end of the key
}
