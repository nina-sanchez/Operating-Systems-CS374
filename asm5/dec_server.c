#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

// defining globals
#define BUFFER 1000
#define PREFIX "D!"
#define TERMINATOR "@"

// functions
void setupAddressStruct(struct sockaddr_in *address, int portNumber);
void encrypt_message(char *input, char *key, char *decipher, long text_length);
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
void encrypt_message(char *input, char *key, char *decipher, long text_length){
    // loop to iterate through the length of the input
    for(int i = 0; i < text_length; i++){
        // starts by doing the arithmetatic
        int input_text = input[i] - 65;
        int input_key = key[i] - 65;
        
        // handling the space character --> ASCII value
        if(input_text == -33){
            input_text = 26;
        }
        // adjusting the value to fit within the range 0-26
        if(input_key == -33){
            input_key = 26;
        }
        // subtracting the key value from the input
        int cipher_key = input_text - input_key;
        
        // if the key is less than 0 then adding in until its in the 0-27 range
        if(cipher_key < 0){
            cipher_key += 27;
        }
        // moding the result to encrypt it
        int result = cipher_key % 27;
        // if its at the end then creating cypher
        if(result == 26){
            decipher[i] = 32;
        } 
        else {
            decipher[i] = result + 65;
        }
    }
}

/*******************************************************
 * function used to handle the client connection
 * does the checkign for the message that its handling
********************************************************/
void handling_client(int connectionSocket){
    // intializing variables needed
    int charsRead, charsWrite;
    char buffer[BUFFER];
    char *error_message = "0";
    // clearing out buffer
    memset(buffer, '\0', BUFFER);
    // reading from the socket until @
    while(strstr(buffer, TERMINATOR) == NULL){ // while it hasnt hit
        memset(buffer, '\0', BUFFER); // clear
        // recieve the info
        charsRead = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
        // checking for prefix
        if(strncmp(buffer, PREFIX, 2) != 0){
            // sending the chars written
            charsWrite = send(connectionSocket, error_message, strlen(error_message), 0);
            // closing the connection
            close(connectionSocket);
            exit(0);
        }
    }

    // remove the newline char
    buffer[strlen(buffer) - 1] = '\0';
    // add 2 to the buffer for mem
    char *temp = buffer + 2;
    // overwriting with what we want
    memmove(buffer, temp, strlen(buffer));

    // calculate the length of buffer
    long text_length = strlen(buffer) / 2;
    // declaring arrays to hold input, key, and decipher
    char input[text_length + 1];
    char key[text_length + 1];
    char decipher[text_length + 2];

    // initialize array with null characters
    memset(input, '\0', text_length + 1);
    // for key
    memset(key, '\0', text_length + 1);
    // for cipher
    memset(decipher, '\0', text_length + 2);

    // copy the first half into the input
    strncpy(input, buffer, text_length);
    // copy the second half into the key
    strcpy(key, &buffer[text_length]);

    // encrypt the input --> call function
    encrypt_message(input, key, decipher, text_length);

    // storing the terminator
    strcat(decipher, TERMINATOR);

    // sending the encrypted input back to the client
    charsWrite = send(connectionSocket, decipher, strlen(decipher), 0);
    if (charsWrite < 0) {
        fprintf(stderr, "ERROR writing to socket\n");
    }
    close(connectionSocket);
    exit(0);
}

/*******************************************************
 * main function used to execute the file, including
 * the ecryption process
********************************************************/
int main(int argc, char *argv[]){
    // initializing variables
    int listenSocket, connectionSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // if less than 2 arguments provided, error message
    if(argc < 2){
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // adapted from course code
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocket < 0){
        fprintf(stderr, "ERROR opening socket\n");
        exit(1);
    }

    // adapted from course code
    setupAddressStruct(&serverAddress, atoi(argv[1]));
    if(bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        fprintf(stderr, "ERROR on binding\n");
        close(listenSocket);
        exit(1);
    }
    listen(listenSocket, 5);

    // inifite loop to handle the child forking
    while(1){
        // adapted from course code
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
        if(connectionSocket < 0){
            fprintf(stderr, "ERROR on accept\n");
        }

        // adapted from course code
        pid_t pid = fork(); // create forking process

        // if less than 0 then error occured
        if(pid < 0){
            // fork error
            fprintf(stderr, "ERROR on fork\n");
        }
        // if is 0 then forking worked properly
        else if(pid == 0){
            close(listenSocket);
            handling_client(connectionSocket);
        }
        else {
            // parent and close
            close(connectionSocket);
        }
    }

    // closing socket
    close(listenSocket);
    return 0;
}
