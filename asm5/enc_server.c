#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

// defining globals
#define BUFFER 1000
#define PREFIX "E!"
#define TERMINATOR "@"

// functions
void setupAddressStruct(struct sockaddr_in *address, int portNumber);
void de_message(char *input, char *key, char *decipher, long text_length);
void handling_client(int connectionSocket);


/*******************************************************
 * adapted from course code, setting up the address
 * structure needed
********************************************************/
void setupAddressStruct(struct sockaddr_in *address, int portNumber){
    memset((char *)address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

/*******************************************************
 * function used to encrypt the message, it works by 
 * using the mod functionality and size of alphabet
 * followed the wikipedia page to see how to do this
********************************************************/
void de_message(char *input, char *key, char *decipher, long text_length){
    // got conversions from course wiki page

    // looping through size of input
    for(int i = 0; i < text_length; i++){
        // converting the capital letters in ASCII
        int input_text = input[i] - 65;
        int input_key = key[i] - 65;

        // if the char is a space, then adjust 
        if(input_text == -33){
            input_text = 26;
        }
        // if the key is a space, then adjust
        if(input_key == -33){
            input_key = 26;
        }
        
        // do the calculations, which is mod 27
        int cipher_key = (input_text + input_key) % 27;
        // converting so that the mod can be space
        if(cipher_key == 26){
            decipher[i] = 32;
        }
        // conversion back to upper case
        else {
            decipher[i] = cipher_key + 65;
        }
    }
}

/*******************************************************
 * function used to handle the client connection
 * does the checkign for the message that its handling
********************************************************/
void handling_client(int connectionSocket){
    // intializing variables needed
    int charsRead, charsWrite, childStatus;
    char buffer[BUFFER];
    memset(buffer, '\0', sizeof(buffer));

    // reading from the socket until @
    while(strstr(buffer, TERMINATOR) == NULL){ // while it hasnt hit
        memset(buffer, '\0', sizeof(buffer)); // clear
        // recieve the info
        charsRead = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
        // if nto all chars sent, error msg
        if(charsRead < 0){
            fprintf(stderr, "ERROR reading from socket\n");
            exit(1);
        }
    }

    // removing the newline
    buffer[strlen(buffer) - 1] = '\0'; 
    // creating temp mem
    char *temp = buffer + 2;
    // overwriting
    memmove(buffer, temp, strlen(buffer));

    // calculating lengths
    long text_length = strlen(buffer) / 2;
    // declaring arrays to hold the input, key, decipher
    char input[text_length + 1];
    char key[text_length + 1];
    char decipher[text_length + 2];

    // initialize array with null characters
    memset(input, '\0', sizeof(input));
    // for key
    memset(key, '\0', sizeof(key));
    // for cipher
    memset(decipher, '\0', sizeof(decipher));

    // copy the first half into the input
    strncpy(input, buffer, text_length);
    // copy the second half into the key
    strcpy(key, &buffer[text_length]);

    // decrypt the input --> call function
    de_message(input, key, decipher, text_length);

    // storing the terminator
    strcat(decipher, TERMINATOR);

    // sending the encrypted input back to the client
    charsWrite = send(connectionSocket, decipher, strlen(decipher), 0);
    // error checking to ensure it all sent
    if (charsWrite < 0){
        fprintf(stderr, "ERROR writing to socket\n");
        exit(1);
    }

    close(connectionSocket);
    exit(0);
}

/***********************************************
 * main function handles all of the functionality
 * will call the needed functions
***********************************************/
int main(int argc, char *argv[]){
    // initializing variables
    int connectionSocket, childStatus;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // checking for arguments
    if(argc < 2){
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // creating socket
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocket < 0){
        fprintf(stderr, "ERROR opening socket\n");
        exit(1);
    }

    // address struct, course code adapted
    setupAddressStruct(&serverAddress, atoi(argv[1]));
    // adapted from source code
    if(bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        fprintf(stderr, "ERROR on binding\n");
        close(listenSocket);
        exit(1);
    }
    // from course code
    listen(listenSocket, 5);

    // infinite loop
    while (1){
        // adapted from course code
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
        if(connectionSocket < 0){
            fprintf(stderr, "ERROR on accept\n");
        }

        // adapted from course code
        // creating fork process
        pid_t pid = fork();

        // if less than 0 then an error occured
        if(pid < 0){
            fprintf(stderr, "ERROR: on forking\n");
        }
        // if 0 then worked correctly
        else if(pid == 0){
            close(listenSocket);
            // will call function to do the neded functionality
            handling_client(connectionSocket);
        }
        else {
            // parent so close
            close(connectionSocket);
        }
    }

    // closing the socket
    close(listenSocket);
    return 0;
}