#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // stat libaries --> referencing replit cited below
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <time.h>
#include <stdbool.h>

// defining --> adapted from split code in canvas
#define PREFIX "movies_"
#define EXTENSION ".csv"
#define PATH_SIZE 1024

/***********************************************************************************************************
** used code from class to help create my code for create movie structure, processing file
** code from class -> https://replit.com/@cs344/studentsc#main.c
** code from class, adapted to learn how to call file with defined prefix, open directory
** and more --> https://replit.com/@cs344/35statexamplec?v=1#main.c
** help me correctly seed rand geneerator --> https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm

** used code from my assignment 1, such as processing movie, creating file executable
************************************************************************************************************/


/****************************************
** Description: Structure for movie
** needed attributes
** reused my code from assignment 1
*****************************************/
struct movie {
    char *title;
    int year;
    char *languages[6];
    float rating_value;
    struct movie *next;
};


/****************************************
** Description: structure to parse
** information about the movie
** I reused my code from assignment 1
*****************************************/
struct movie createMovie(char *curr_line){
    struct movie *curr_movie = malloc(sizeof(struct movie)); //allocate mem
    char *save_ptr;

    // first token is title
    char *token = strtok_r(curr_line, ",", &save_ptr);
    curr_movie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(curr_movie->title, token);

    // second token is year
    token = strtok_r(NULL, ",", &save_ptr);
    sscanf(token, "%d", &curr_movie->year);

    // third token is language
    token = strtok_r(NULL, ",", &save_ptr);
    char *temp = calloc(strlen(token) + 1, sizeof(char));
    int iterate = 0;
    char *language_char = NULL;
    for(int i = 0; i < strlen(token); i++){ // for loop to get languages in brackets
        if(token[i] != '[' && token[i] != ']'){
            temp[iterate++] = token[i];
        }
    }
    
    language_char = temp;
    // initialize languages array to NULL
    for(int i = 0; i < 6; i++){  // at most will have 5 languages
        curr_movie->languages[i] = NULL;
    }
    // each language
    int num_languages = 0;
    char *spacing = NULL;
    while((spacing = strtok_r(temp, ";", &temp)) != NULL && num_languages < 6){
        curr_movie->languages[num_languages] = calloc(strlen(spacing) + 1, sizeof(char));
        strcpy(curr_movie->languages[num_languages++], spacing);
    }

    // fourth token is rating value
    token = strtok_r(NULL, ",", &save_ptr);
    sscanf(token, "%f", &curr_movie->rating_value);

    curr_movie->next = NULL;
    return *curr_movie;

};


/****************************************
** Description: returns linked list of
** movie by parsing information
** re used my code from assignment 1
*****************************************/
struct movie *process_file(char *filePath){
    FILE *movie_file = fopen(filePath, "r");
      
    if(movie_file == NULL){ //base case
        perror("error...");
        exit(1);
    }
      
    // intializing
    char *curr_line = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;
    int movies;

    // head of the linked list
    struct movie *head = NULL;
    // tail of the linked list
    struct movie *tail = NULL;

    // read file line by line
    while((nread = getline(&curr_line, &len, movie_file)) != -1){
    // get new movie node corresponding to the curr line
        struct movie *new_node = malloc(sizeof(struct movie));
        *new_node = createMovie(curr_line);
        movies++;

        // is this the first node in the ll?
        if(head == NULL){
            head = new_node;
            tail = new_node;
        }
        else{
            tail->next = new_node;
            tail = new_node;
        }
    }
    
    movies = movies - 1;  // ensures to remove top line of file
    //printf("Processed file %s and parsed data for %d movies\n", filePath, movies); // prints message 
    free(curr_line);
    fclose(movie_file);
    return head;
};


/*************************************************
** function create txt files with movie titles in it
** adapted structure from class replit code
** https://replit.com/@cs344/35statexamplec#main.c
**************************************************/
void movie_text_file(char *file_path, char *directory_path){
    FILE *movie_file = fopen(file_path, "r"); //opens file
    if(movie_file == NULL){
        perror("error...");
        exit(1);
    }

    char *curr_line = NULL;
    size_t length = 0;
    ssize_t nread; // getline
    getline(&curr_line, &length, movie_file);

    while((nread = getline(&curr_line, &length, movie_file)) != -1){ // loops until reaches end, since getline using -1
        int file_num = 0;
        struct movie node = createMovie(curr_line); 
        char file_path_year[PATH_SIZE]; // create path in the directory
        sprintf(file_path_year, "%s/%d.txt", directory_path, node.year);
        // octal representation: rw- = 6, r-- = 4, --- = 0 
        // O_CREAT = creates file, wronly = write access, and append will make sure added to bottom of files
        file_num = open(file_path_year, O_CREAT | O_WRONLY | O_APPEND, 0640); // adapted permissions from class replit -> https://replit.com/@cs344/35openclosec#main
        if(file_num == -1){ // base case
            perror("error...");
            exit(1);
        }

        char sending_title[PATH_SIZE];
        sprintf(sending_title, "%s\n", node.title); // writing to file
        ssize_t string = write(file_num, sending_title, strlen(sending_title));
        close(file_num);
    }

    free(curr_line); // freeing memory
    fclose(movie_file); //closing file
}


/*************************************************
** function to find file in directory with smallest size
** adapted structure from class replit code
** https://replit.com/@cs344/35statexamplec#main.c
**************************************************/
void *maximum_file_size(){
    DIR* currDir = opendir("."); // open current directiry
    if(currDir == NULL){ // if nothing in directiry
        perror("error..."); // error message
        exit(1);
    }

    struct dirent *aDir; // using replit structure from class
    struct stat dirStat; // data from directory
    char file_path[PATH_SIZE];
    off_t file_size = 0;
    char *maximum_file_size = malloc(sizeof(char) * PATH_SIZE); // allocating memory

    while((aDir = readdir(currDir)) != NULL){ // go through all entries
        if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 && 
        strstr(aDir->d_name, EXTENSION) != NULL){ // get data for entry
            snprintf(file_path, PATH_SIZE, "%s/%s", ".", aDir->d_name); // new line to not make so long
            if(stat(file_path, &dirStat) == 0){ //getting data --> adapted from replit
                if(dirStat.st_size >= file_size){ // storing biggest file, st_size = bytes
                    file_size = dirStat.st_size;
                    strcpy(maximum_file_size, aDir->d_name);
                }
            }
            else { // error handling
                perror("error...");
                exit(1);
            }
        }
    }

    closedir(currDir); // close directory 
    return maximum_file_size;  // returns findings
}


/*************************************************
** function to find file in directory with smallest size
** adapted structure from class replit code
** https://replit.com/@cs344/35statexamplec#main.c
**************************************************/
char *minimum_file_size(){
    DIR* currDir = opendir("."); // open current directiry
    if(currDir == NULL){ // if nothing in directiry
        perror("error..."); // error message
        exit(1);
    }

    struct dirent *aDir; // using replit structure from class
    struct stat dirStat; // data from directory
    char file_path[PATH_SIZE];
    off_t file_size = LONG_MAX;
    char *minimum_file_sizes = NULL; //pointer null
    while((aDir = readdir(currDir)) != NULL){ // while stuff in directory
        if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 && // broke up for readability
            strstr(aDir->d_name, EXTENSION) != NULL){ // get data for entry)
            snprintf(file_path, PATH_SIZE, "%s/%s", ".", aDir->d_name);
            if(stat(file_path, &dirStat) == 0){
                if(minimum_file_sizes == NULL || dirStat.st_size < file_size){ // kept get seg faults
                    if(minimum_file_sizes != NULL){ // freeing mem
                        free(minimum_file_sizes);
                    }
                    minimum_file_sizes = malloc(sizeof(char) * PATH_SIZE); // allocating memory
                    if(minimum_file_sizes == NULL){
                        perror("seg - testing"); //testing - worked
                        exit(1);
                    }
                    file_size = dirStat.st_size; // finding min
                    strcpy(minimum_file_sizes, aDir->d_name); // 
                }
            }
            else {
                perror("seg seg"); // testing - worked
                exit(1);
            }
        }
    }

    closedir(currDir); // closing directory
    return minimum_file_sizes;
}


/*************************************************
** created the random seed generator, lists processing
** file, created title name + sets the directory 
** rules
** adapted random num generator --> cited at top of file
** adapted from course code
**************************************************/
void directory_path(char *curr_line){
    char dir_title[PATH_SIZE]; 
    char dir_path[PATH_SIZE];
    char file_path[PATH_SIZE];
    int n, random_number; // initialize
    time_t t;
    srand((unsigned) time(&t)); // used this because first run kept being same ran numbers
    random_number = rand() % 99999; // chooses between given range
    printf("Now processing the chosen file named %s\n", curr_line);
    sprintf(dir_title, "sanchreg.movies.%d", random_number);
    int creation = mkdir(dir_title, 0750); // octal representation of permission, rwx = 7, r-x = 5, --- = 0
    printf("Created directory with name %s\n", dir_title);
    sprintf(dir_path, "./%s", dir_title); // storing its path
    sprintf(file_path, "./%s", curr_line);
    movie_text_file(file_path, dir_path); // creates the file

}


/*************************************************
** file allows users to pick file of their picking
** adapted from canvas split code, cited at top 
**************************************************/
void pick_file_name(char **file_names) {
    char input_name[PATH_SIZE]; // initalizing, opening directory
    char file_path[PATH_SIZE]; 
    DIR *currDir = opendir(".");
    struct dirent *aDir;
    printf("Enter the name of the file: ");
    scanf("%s", input_name);

    // allocating memory, storing info
    *file_names = malloc(sizeof(char) * (strlen(input_name) + 1));
    strcpy(*file_names, input_name);
    snprintf(file_path, PATH_SIZE, "./%s", *file_names);

    while ((aDir = readdir(currDir)) != NULL) { // while stuff is in directory
        if (strcmp(aDir->d_name, *file_names) == 0) {
            printf("Selected file: %s\n", *file_names);
            closedir(currDir); // close directory
            directory_path(*file_names); // create path
            return;
        }
    }

    if((aDir = readdir(currDir)) == NULL){ // if no file in directory matches user name
        printf("The file %s was not found..\n", *file_names); // prints error + file name
    }
    // freeing memory
    free(*file_names);
    *file_names = NULL; // free pointer
}


/****************************************
** Description: prints main menu for users
** to see their options or exit program
*****************************************/
void user_file_options(){
    printf("------------------------------------------------------------------------------\n");
    printf("1. Select file to process\n");
    printf("2. Exit from the program\n");
    printf("------------------------------------------------------------------------------\n");
    printf("Enter a choice of 1 or 2: ");
}


/****************************************
** Description: prints menu for users
** to see their options
*****************************************/
void user_options(){
    printf("------------------------------------------------------------------------------\n");
    printf("Which file do you want to process?\n");
    printf("1. Pick the largest file\n");
    printf("2. Pick the smallest file\n");
    printf("3. Specify the name of a file\n");
    printf("------------------------------------------------------------------------------\n");
    printf("Enter a choice from 1 to 3: ");

}


/****************************************
** Description: do while loop to go through
** different user wants
*****************************************/
void menu_option_change(){
    int user_option;
    char *file_name;

    do{
        user_options();
        scanf("%d", &user_option);
        if(user_option == 1){
            //printf("option 1\n");
            file_name = maximum_file_size();
            directory_path(file_name);

        }
        else if(user_option == 2){
            //printf("option 2\n");
            //char *files = NULL;
            char *files = minimum_file_size();
            // file_name = minimum_file_size();
            directory_path(files);
            //printf("seg 3");
        }
        else if(user_option == 3){
            //printf("option 3\n");
            // pick_file_name(file_name);
            // directory_path(file_name);
            char *file_names = NULL;
            pick_file_name(&file_names);
            //directory_path(file_names);

        }

        if(user_option > 3 || user_option < 1){
            printf("error... try again\n");
        }

    }while(user_option != 1 && user_option != 2 && user_option != 3); // when selects one of the menu options it will break out of loop
}

/****************************************
** Description: main function to execute
** program
** compile with: gcc --std=gnu99 -o movies_by_year movies_by_year.c
*****************************************/
int main(int argc, char *argv[]){
    int option; 
    char *fileName;

    //struct movie *list = process_file(fileName);
    /* TESTING */

    do{
        user_file_options();
        scanf("%d", &option);
        if(option == 2){
                printf("bye!\n");
                return 1;
        }
        else if(option == 1){
            //printf("option 1\n");
            menu_option_change();
        }

        if(option > 2 || option < 1){ // error 
            printf("error... try again\n\n");
         }

    }while(option != 2);

    return EXIT_SUCCESS;
}





