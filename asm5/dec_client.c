#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>      
#include <netinet/in.h>

// defining globals
#define BUFFER 1000
#define PREFIX "D!"
#define TERMINATOR "@"

// calling all of my functions
void error(const char* message);
void setupAddressStruct(struct sockaddr_in *address, int portNumber, char *hostname); // given from course code
char* read_file(const char* filename, long* length);
void process_input(char *input, long *input_length, char *argv[]);
void process_key(char *key, long *key_length, long input_length, char *argv[]);
void handle_socket_errors(int charsWritten, const char* buffer);
void receive_data(int socketFD, char* input_buffer, char* buffer, const char* port_number);

/*************************************************
 * error function to call if there is an error
 * adapted code from course material
*************************************************/
void error(const char* message){
    perror(message);
	exit(0);
}

/*************************************************
 * setting up the address struct
 * adapted code from course material
*************************************************/
void setupAddressStruct(struct sockaddr_in *address, int portNumber, char *hostname){
    memset((char *)address, '\0', sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    struct hostent *hostInfo = gethostbyname(hostname);
    
    if(hostInfo == NULL){
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(1);
    }

    memcpy((char *)&address->sin_addr.s_addr,
           hostInfo->h_addr_list[0],
           hostInfo->h_length);
}

/*************************************************
 * function that reads in information from the 
 * files, called in main and just change the argv
 * to select what file is getting read in
*************************************************/
char* read_file(const char* filename, long* length){
    // opening the file to read
    FILE *file_input = fopen(filename, "r");
   
    // if the file is empty --> error checking
    if(file_input == NULL){
        fprintf(stderr, "CLIENT: error opening text file %s\n", filename);
        exit(1);
    }

    // gets the file input if exists
    fseek(file_input, 0, SEEK_END);
    // gets the length of the file input
    *length = ftell(file_input);
    fseek(file_input, 0, SEEK_SET);

    // allocating memory, using buffer
    char* buffer = malloc(*length + 1);
    // if there wasnt any buffer mem being allocated
    if(buffer == NULL){
        fclose(file_input);
        // then error occured
        fprintf(stderr, "memory allocation failed\n");
        exit(1); // exits
    }

    // reads the info with allocated buffer size
    fread(buffer, 1, *length, file_input);
    // closing the file being read
    fclose(file_input);
    // clear out the buffer
    buffer[*length] = '\0';

    // return the buffer
    return buffer;  
}

/*************************************************
 * error checking the input to ensure it is valid
 * will exit if contains bad character
*************************************************/
void process_input(char *input, long *input_length, char *argv[]){
    // removing new line
    if(*input_length > 0){
        input[*input_length - 1] = '\0';
        (*input_length)--;
    } 
    // if the input size is less than 0 then error
    else {
        fprintf(stderr, "ERROR: input is empty\n");
        exit(1);
    }

    // looping thruogh input to check for bad characters
    for(int i = 0; i < *input_length; i++){
        int characters = input[i];
        // if it is less than 64, and not 32, or greater than 90, going through ASCII
        if((characters < 65 && characters != 32) || characters > 90){
            // if its not within the ascii wanted then will output an error message
            fprintf(stderr, "%s ERROR: input contains bad characters\n", argv[0]);
            exit(1);
        }
    }
}

/*************************************************
 * checking if the key is valid
 * exits if it is not a valid key
*************************************************/
void process_key(char *key, long *key_length, long input_length, char *argv[]){
    // if key exits
    if(*key_length > 0){
        // remove new line
        key[*key_length - 1] = '\0';
        // remove length
        (*key_length)--;
    } 
    else {
        // else key doesnt exist
        fprintf(stderr, "ERROR: key doesn't exist\n");
        exit(1);
    }

    // error checking input size vs key
    if(*key_length < input_length){
        fprintf(stderr, "ERROR: key is too short\n");
        exit(1);
    }
}

/*************************************************
 * this function handles the errors that could occur when
 * writing to the socket, will check the output buffer
*************************************************/
void handle_socket_errors(int charsWritten, const char* buffer){
    // if the characters writen to socket is less than 0 then nothing sent
    if(charsWritten < 0){
        // prints error
        fprintf(stderr, "CLIENT: error writing to socket\n");
    }
    // if the chars written are less than mem allocated for buffer
    if(charsWritten < strlen(buffer)){
        // then not everything sent so error message
        fprintf(stderr, "CLIENT: not all data written to socket!\n");
    }
}

/**************************************************
 * this is getting in the data, doing error checking
 * and checking the buffer
**************************************************/
void receive_data(int socketFD, char* input_buffer, char* buffer, const char* port_number){
    // while it is still reading, comparing strings
    while(strstr(input_buffer, "@") == NULL){
        // clearing out input
        memset(input_buffer, '\0', BUFFER);
        
        // receiving from socket
        int charsRead = recv(socketFD, input_buffer, BUFFER - 1, 0);

        // if the characters read is less than 0, then error
        if(charsRead < 0){
            fprintf(stderr, "CLIENT: ERROR reading from socket\n");
            exit(1);
        }
        // if the input is 0
        if(strstr(input_buffer, "0")){
            // error
            fprintf(stderr, "Error: could not contact dec_server on port %s\n", port_number);
            exit(2);
        }

        // storing
        strcat(buffer, input_buffer);

        // breaking out of loop
        if(strstr(buffer, "0")){
            break;
        }
    }
}

/*************************************************
 * main function will execute all of the code
 * 
*************************************************/
int main(int argc, char *argv[]) {
    // initializing variables
    char *input;
    long input_length;
    char *key;
    long key_length;
    int socketFD, charsWritten, charsRead;
    struct sockaddr_in serverAddress;

    // if user enters less than 4 arguments
    if(argc < 4){
        // prints error and exits
        fprintf(stderr, "USAGE: %s input_file key_file port_number\n", argv[0]);
        exit(1);
    }

    // this is processing the input file, calling function to read it in
    input = read_file(argv[1], &input_length);
    process_input(input, &input_length, argv); // error checking the inputed info

    // this is processing the key file, calling function to read it in
    key = read_file(argv[2], &key_length);
    // calling function to process the key, error checking + cleaning it up
    process_key(key, &key_length, input_length, argv);

    // creating output buffer size, adding in isze for the end + beginning 
    long output = (input_length * 2) + 4;
    char *buffer = malloc(output);
    
    // storing copy in output buffer to send corrrectly
    // this is storing the prefix to show beginning of message
    strcpy(buffer, PREFIX);
    // storing the actual input
    strcat(buffer, input);
    // combining
    strncat(buffer, key, input_length);
    // appending the terminator to show end of message
    strcat(buffer, TERMINATOR);

    // adapted from course code
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    
    // error checking socket
    if(socketFD < 0){
        fprintf(stderr, "CLIENT: ERROR opening socket\n");
    }

    // adapted from course code
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // adapted from course code
    if(connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        fprintf(stderr, "CLIENT: ERROR connecting socket\n");
    }

    // sending the message
    charsWritten = send(socketFD, buffer, strlen(buffer), 0);
    // error checking the handling
    handle_socket_errors(charsWritten, buffer); 

    // clearing out
    memset(buffer, '\0', output);
    char input_buffer[BUFFER];

    // getting the data
    receive_data(socketFD, input_buffer, buffer, argv[3]);

    // clearing
    buffer[strlen(buffer) - 1] = '\n';
    fprintf(stdout, "%s", buffer);

    // closing the connection
    close(socketFD);
    return 0;
}
