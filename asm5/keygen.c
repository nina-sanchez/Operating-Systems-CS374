#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// defining globals
#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZ "

/****************************************************
 * the main function to execute the creation
 * of keys 
****************************************************/
int main(int argc, char *argv[]){
    
    int keylength;

    // first want to make sure enough arguments
    if(argc < 2){
        fprintf(stderr, "not enough arguments...\n"); 
        exit(0); 
    }

    keylength = atoi(argv[1]); // converting
    if(keylength <= 0){ // error checking that its a valid input
        fprintf(stderr, "error... invalid key length\n");
        return 1;
    }

    char key[keylength + 1]; //creating var key to have space for keylength

    srand(time(NULL)); // seed random numbers
    
    int i;
    for(i = 0; i < keylength; i++){ // loops through the keylength
        int num_chars = sizeof(ALPHABET) - 1; //number of characters is size of alphabet
        key[i] = ALPHABET[rand() % num_chars]; //puts rando num into key
    }

    key[i] = '\n'; 
    key[i + 1] = '\0'; // new line at the end
    printf("%s", key); // prints the key

    return 0;
}