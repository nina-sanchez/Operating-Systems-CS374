#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>
#include <errno.h>

#define COMMAND_LINE_MAX 2048
#define ARGUMENT_MAX 512
#define ARGUMENT_LENGTH 50

// constant intialization
int background = 0;

// all of my functions
void replace_pid(char *arguments);
void token_user_input(char *user_input[], char input_file[], char output_file[], int *background_flag);
void user_cd(char *user_input[]);
void user_exit();
void user_status();
void controlc_handler(int signo);
void controlz_handler(int signo);
void other_commands(char *user_input[], char input_file[], char output_file[], int *background_flag);

/**************************************************************
** this function will replace $$ from user input and replace
** it with their pid id. the function is called in the
** token user input
**************************************************************/
void replace_pid(char *arguments){
    // will be comparing initial arguments with $$
    char *user_pid = strstr(arguments, "$$");
    
    while(user_pid != NULL){
        // getting length
        size_t initial_string = user_pid - arguments;
        // length after the pid
        size_t changed_string = strlen(user_pid + 2);
        // mem for new string which will have the pid in #'s
        char *new_string = (char *)malloc(initial_string + sizeof(pid_t) + changed_string + 1);
        strncpy(new_string, arguments, initial_string);
        // getting user pid #
        pid_t pid = getpid();
        // insert into string
        sprintf(new_string + initial_string, "%d", pid);
        strcat(new_string, user_pid + 2);
        strcpy(arguments, new_string);
        // free the allocated mem
        free(new_string);
        // see if there's another set of $$ in string
        user_pid = strstr(arguments, "$$");
    }
}

/**************************************************************
** this function will essentially tokenize the different parts
** of the user input. It will store input file name, output file
** name, and create flag for background mode
**************************************************************/
void token_user_input(char *user_input[], char input_file[], char output_file[], int *background_flag) {
    char arguments[COMMAND_LINE_MAX];
    int i = 0; // index for user_input
    //*background_flag = 0;
    // get user input
    fgets(arguments, COMMAND_LINE_MAX, stdin);

    // when the user hits enter, remove the newline
    arguments[strcspn(arguments, "\n")] = '\0';
    //calling function
    replace_pid(arguments); // calling function to replace pid from '$$'

    // added to clear previous user input, no tokens carry over
    memset(user_input, 0, sizeof(char *) * ARGUMENT_MAX);
    memset(input_file, 0, sizeof(char *) * ARGUMENT_MAX);
    memset(output_file, 0, sizeof(char *) * ARGUMENT_MAX);
    
    // setting token to space
    char *token = strtok(arguments, " ");

    // while more tokens
    while(token != NULL) {
        // first store input file name is string has '<'
        if(strcmp(token, "<") == 0){
            token = strtok(NULL, " ");
            if(token != NULL){
                strcpy(input_file, token);
                //printf("file input: %s\n", input_file);
            } 
            else {
                printf("error.. no file name\n");
                fflush(stdout);
            }
        } 
        // second store if string has '>'
        else if(strcmp(token, ">") == 0){
            token = strtok(NULL, " ");
            if(token != NULL){
                strcpy(output_file, token);
                //printf("file output: %s\n", output_file);
            } 
            else {
                printf("error... no file name\n");
                fflush(stdout);
            }
        } 
        // third flag background if string has &
        else if(strcmp(token, "&") == 0){
            *background_flag = 1;
            //printf("Background flag = 1\n");
            fflush(stdout);
        } 
        // allocate mem for input and copy token to it
        else {
            user_input[i] = strdup(token);
            i++;
        }
        // continue
        token = strtok(NULL, " ");
    }

    // trying to set flag to 0 if wasnt flagged --> i think
    // background flag is part of problem i have
    // if(*background_flag != 1){
    //     *background_flag = 0;
    // }


}

/**************************************************************
** this function will change the directory of user input.
** if there is a second argument that is the cd name so move
** to that, if not cd to home
**************************************************************/
void user_cd(char *user_input[]){
    char curr_dir[COMMAND_LINE_MAX];

    // if there is an arg after cd:
    if(user_input[1] != NULL){
        if(chdir(user_input[1]) == -1){
            perror("error...\n");
            printf("couldn't change to directory %s\n", user_input[1]);
            fflush(stdout);
        }
    }
    // if not arg after cd
    else {
        if(chdir(getenv("HOME")) == -1){
            perror("error...");
        }
    }

}

/**************************************************************
** this function will exit program, called in main function
** 
**************************************************************/
void user_exit(){
    exit(0);
}

/**************************************************************
** this function will replace $$ from user input and replace
** it with their pid id. the function is called in the
** token user input
// adapted code from module: https://canvas.oregonstate.edu/courses/1954185/pages/exploration-permissions?module_item_id=23945178
// used another canvas module but cant find the module i looked at
**************************************************************/
void user_status(){
    int exit_status = 0;
    if(WIFEXITED(exit_status)){
            printf("exit value %d\n", WIFEXITED(exit_status));
            fflush(stdout);
        } 
        else if(WIFSIGNALED(exit_status)){
            printf("terminated by signal %d\n", WTERMSIG(exit_status));
            fflush(stdout);
        } 
        else {
            printf("error...\n");
            fflush(stdout);
        }

}

/**************************************************************
** this function will handle the control c inputs
** if user inputs, will sleep for 5 seconds
** code adapted from canvas module
//https://canvas.oregonstate.edu/courses/1954185/pages/exploration-signal-handling-api?module_item_id=23945193
**************************************************************/
void controlc_handler(int signo){
    char *message = "Caught SIGINT, sleeping for 5\n";
    write(STDOUT_FILENO, message, strlen(message));
    sleep(5);
}

/**************************************************************
** this function will handle the control z inputs
** if background flag is 1, then it will exit foreground
** if background = 0, it will enter foreground mode
** code adapted from canvas module
//https://canvas.oregonstate.edu/courses/1954185/pages/exploration-signal-handling-api?module_item_id=23945193
**************************************************************/
void controlz_handler(int signo){
    if(background == 1){
        char *message = "exiting foreground-only mode\n";
        write(STDOUT_FILENO, message, strlen(message));
        fflush(stdout);
        background = 0;
    }
    // if(background == 0){
    else {
        char *message_two = "entering foreground-only mode\n";
        //printf("entering foreground-only mode\n");
        write(STDOUT_FILENO, message_two, strlen(message_two));
        fflush(stdout);
        background = 1;
    }
}

/**************************************************************
** this function will handle the not built in user inputs
** it works by using a switch case and having child execute the different
** commands, it also deals with the file i/o
** code adapted from canvas module:
* https://canvas.oregonstate.edu/courses/1954185/pages/exploration-processes-and-i-slash-o?module_item_id=23945194
* https://canvas.oregonstate.edu/courses/1954185/pages/exploration-permissions?module_item_id=23945178
* https://canvas.oregonstate.edu/courses/1954185/pages/exploration-process-concept-and-states?module_item_id=23945183
* https://replit.com/@cs344/4forkexamplec#main.c
**************************************************************/
void other_commands(char *user_input[], char input_file[], char output_file[], int *background_flag){
    int exit_status = 0;
    // struct sigaction SIGINT_action = {0};
    // struct sigaction SIGINT_action;
    struct sigaction SIGINT_action = {0};
    // SIGINT_action.sa_handler = controlc_handler;
    // sigfillset(&SIGINT_action.sa_mask);
    // SIGINT_action.sa_flags = 0;
    // sigaction(SIGINT, &SIGINT_action, NULL);

    // control z handler --> from canvas page
    struct sigaction SIGTSTP_action = {0};
    // SIGTSTP_action.sa_handler = controlz_handler;
    // sigfillset(&SIGTSTP_action.sa_mask);
    // SIGTSTP_action.sa_flags = 0;
    // sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    pid_t spawnpid = -5; // adapted from canvas
    spawnpid = fork();

    switch(spawnpid){
        case -1:
            perror("fork () failed");
            exit(1);
            break;
        
        case 0:
            // sigint
            SIGINT_action.sa_handler = SIG_DFL;
            sigaction(SIGINT, &SIGINT_action, NULL);
            
            // if input file isnt empty
            if(strcmp(input_file, "")){
                int temp = 0;
                temp = open(input_file, O_RDONLY);
                //if error
                if(temp == -1){
                    printf("cannot open %s file\n", input_file);
                    fflush(stdout);
                    exit(1);
                }
                dup2(temp, 0);
                close(temp);

            }
            // if output file isnt empty
            if(strcmp(output_file, "")){
                int temp_two = 0;
                temp_two = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                // if error
                if(temp_two == -1){
                    printf("cannot send to file %s\n", output_file);
                    fflush(stdout);
                    exit(1);
                }
                dup2(temp_two, 1);
                close(temp_two);
            }

            // execute / error handling
            if(execvp(user_input[0], user_input)){
                perror(user_input[0]);
                fflush(stdout);
                exit(1);
            }
            break;

        default:
            // need to fix this whole thing
            if(*background_flag == 1 && background == 0){
                pid_t childPid = waitpid(spawnpid, &exit_status, WNOHANG);
                printf("background pid: %d\n", spawnpid);
                fflush(stdout);
                *background_flag = 0;
            }
            else {
                pid_t childPid = waitpid(spawnpid, &exit_status, 0);
            }
            // exit_status = 0;
        while((spawnpid = waitpid(-1, &exit_status, WNOHANG)) > 0){
                printf("background process %d ", spawnpid);
                fflush(stdout);
            // getting exit value --> essentially same as user status
            if(WIFEXITED(exit_status)){
                int temp_status = WEXITSTATUS(exit_status);
                printf("exit value %d\n", temp_status);
                fflush(stdout);
            } // getting terminated signal
            else {
                int temp_status_two = WTERMSIG(exit_status);
                printf("terminated by signal %d\n", temp_status_two);
                fflush(stdout);
            }
        }
        break;
    }
}


// followed in class pseudocode to create my main
int main(int argc, char *argv[]){
    // initializing
    char *user_input[ARGUMENT_MAX];
    char input_file[COMMAND_LINE_MAX];
    char output_file[COMMAND_LINE_MAX];
    int background_flag = 0;
    
    // control c handler --> from canvas page
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = controlc_handler;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // control z handler --> from canvas page
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = controlz_handler;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
    
    // infinite loop
    while(1){
        //printf("%d: ", getpid());
        background_flag = 0;
        //background = 0;
        printf(": ");
        fflush(stdout);
        //tokenize the user input + store info the parameters
        token_user_input(user_input, input_file, output_file, &background_flag);
        //if not empty, if have #, or new line, continue
        if(user_input[0] != NULL && user_input[0][0] == '#' | user_input[0][0] == '\0'){
            continue;
        }
        // if user input is cd, then go to that func
        else if(strcmp(user_input[0], "cd") == 0){
            //printf("cd..\n");
            user_cd(user_input);
        }
        // if user input is status, go to that func
        else if(strcmp(user_input[0], "status") == 0){
            //printf("status\n");
            user_status();

        }
        // if user input is exit, exit program
        else if(strcmp(user_input[0], "exit") == 0){
            user_exit();

        }
        // all other commands executed by parent + child
        else {
            //printf("exec commands\n");
            //printf("input file in main: %s\n", input_file);
            other_commands(user_input, input_file, output_file, &background_flag);

        }
    }

    return 0;
}