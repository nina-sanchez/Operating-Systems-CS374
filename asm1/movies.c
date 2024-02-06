#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// used code from class to help create my code for create movie structure
// processing file
// code from class -> https://replit.com/@cs344/studentsc#main.c


/****************************************
** Description: Structure for movie
** needed attributes
**
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
**
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
**
*****************************************/
struct movie *process_file(char *filePath){
      FILE *movie_file = fopen(filePath, "r");
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
      while ((nread = getline(&curr_line, &len, movie_file)) != -1){
      // get new movie node corresponding to the curr line
            struct movie *new_node = malloc(sizeof(struct movie));
            *new_node = createMovie(curr_line);
            movies++;

            // is this the first node in the ll?
            if (head == NULL){
                  head = new_node;
                  tail = new_node;
            }
            else{
                  tail->next = new_node;
                  tail = new_node;
            }
      }
    
      movies = movies - 1;  // ensures to remove top line of file
      printf("Processed file %s and parsed data for %d movies\n", filePath, movies); // prints message 
      free(curr_line);
      fclose(movie_file);
      return head;
};


/****************************************
** Description: prints information
** on the movies
** * used for testing
*****************************************/
void print_movie(struct movie* a_movie){
      printf("%s, %d, %s, %.1f\n\n", a_movie->title,
                        a_movie->year,
                        a_movie->languages,
                        a_movie->rating_value);
};

/****************************************
** Description: prints movie by the
** year it was released
**
*****************************************/
void print_movie_year(struct movie* list){
      int user_input;
      bool movie_exists = false;  // setting bool so can give error message

      printf("Enter a year: ");
      scanf("%d", &user_input);

      while(list != NULL){  // will continue to print until no more movies in list
            if(list->year == user_input){
                  printf("%s\n", list->title);
                  movie_exists = true;
            }
            list = list->next;
      }

      if(movie_exists == false){  // if no movie released in that year
            printf("No data about movies released in the year ");
            printf("%d\n", user_input);
      }
}

/****************************************
** Description: prints movie if it is
** released in given year, input given
** by the user
*****************************************/
void print_specific_language(struct movie* list){
      char user_input[50]; // caused seg fault not allowing set chars
      bool checker = false;  // setting bool flag
      printf("Enter the language for which you want to see movies: ");
      scanf("%s", user_input);

      while(list != NULL){
            for(int i = 0; i < 6; i++){ // will be at most 5 languages
                  if(list->languages[i] != NULL && strcmp(list->languages[i], user_input) == 0){
                        printf("%d %s\n", list->year, list->title);
                        checker = true;
                  }
            }
            list = list->next;
      }
      if(checker == false){ // error message if no matching language
            printf("No data about movies released in ");
            printf("%s\n", user_input);
      }
}


/****************************************
** Description: prints movies by highest
** rating value for the given year, no
** repeated years
*****************************************/
void print_rating_value(struct movie* list){
      struct movie* current = list; //original list
      struct movie rated_highest;
      float rating = 0;
      for(int i = 1900; i < 2022; i++){ // loop to iterate through the years 1900 -> 2022
            list = current;
            while(list != NULL){
                  if(list->year == i){
                        if(list->rating_value > rating){ // will get highest rating
                              rating = list->rating_value;
                              rated_highest = *list;
                        }
                  }
                  list = list->next;
            }
            if(rating > 0){
                  printf("%d %.1f %s\n", rated_highest.year, rating, rated_highest.title);
                  rating = 0;
            }
      }
}

/****************************************
** Description: prints all movies
** in the list
** * used for testing
*****************************************/
void print_movie_list(struct movie *list){
      while (list != NULL){
            print_movie(list);
            list = list->next;
      }
}

/****************************************
** Description: prints menu for users
** to see their options
**
*****************************************/
void user_options(){
      printf("------------------------------------------------------------------------------\n");
      printf("1. Show movies released in the specified year\n");
      printf("2. Show highest rated movie for each year\n");
      printf("3. Show the title and year of release of all movies in a specific language\n");
      printf("4. Exit from the program\n");
      printf("------------------------------------------------------------------------------\n");
      printf("Enter a choice from 1 to 4: ");

}

/****************************************
** Description: main function to execute
** program
** compile with: gcc --std=gnu99 -o movies movies.c
*****************************************/
int main(int argc, char *argv[]){
      int option; 

      if(argc < 2){
            printf("You must provide the name of the file to process\n");
            printf("Example usage: ./movies movies_sample_1.csv\n");
            return EXIT_FAILURE;
      }

      struct movie *list = process_file(argv[1]);
      
      // testing - 
      //print_file_executed(list);
      //movie_program();
      //print_movie_list(list);

      do{  // used do while loop, breaks when user enters 4
            user_options();
            scanf("%d", &option);
            if(option == 4){
                  printf("bye\n");
                  return 1; // breaks
            }
            else if(option == 1){
                  print_movie_year(list);
            }
            else if(option == 2){
                  print_rating_value(list);
            }
            else if(option == 3){
                  print_specific_language(list);
            }
            if(option > 4 || option < 1){  // error checking for int values
            printf("Error... try again.\n");
            }

      }while(option !=4);

    return EXIT_SUCCESS;
}





