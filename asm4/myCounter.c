#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// initializers and globals
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t myCond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t myCond2 = PTHREAD_COND_INITIALIZER;
int myCount = 0;
int buffer = 0;

// functions
void* producer(void* argument);
void* consumer(void* argument);

// adapted code from the slides + modules


/***********************************************
** producer function which prints count,
** sends and recieves signal from consumer
***********************************************/
void* producer(void* argument){
    while(1){
        // lock the mutex
        printf("PRODUCER: myMutex locked\n");
        fflush(stdout);
        pthread_mutex_lock(&myMutex);

        // this means buffer is full so so waits for consumer
        while(buffer == 1){
            printf("PRODUCER: waiting on myCond1\n");
            fflush(stdout);
            pthread_cond_wait(&myCond1, &myMutex);
        }

        // increment buff, inc my count
        buffer++;
        myCount++;

        printf("myCount: %d -> %d\n", myCount - 1, myCount);
        fflush(stdout);

        //unlock mutexxx
        printf("PRODUCER: myMutex unlocked\n");
        fflush(stdout);
        pthread_mutex_unlock(&myMutex);

        // tell consumer --> cond ready
        printf("PRODUCER: signaling myCond2\n");
        fflush(stdout);
        pthread_cond_signal(&myCond2);

        // breaks loop so stops at 10
        if(myCount >= 10){
            break;
        }
    }

    return NULL;
}

/***********************************************
** consumer function which prints count,
** sends and recieves signal from producer
***********************************************/
void* consumer(void* argument){
    while(1){
        // lock mutexx
        printf("CONSUMER: myMutex locked\n");
        fflush(stdout);
        pthread_mutex_lock(&myMutex);

        // this means buffer is empty so waiting for item from prod
        while(buffer == 0){
            printf("CONSUMER: waiting on myCond2\n");
            fflush(stdout);
            pthread_cond_wait(&myCond2, &myMutex);
        }

        // then bounces to 0, will alt 0 and 1
        buffer--;   

        //unlock mutex
        printf("CONSUMER: myMutex unlocked\n");
        fflush(stdout);
        pthread_mutex_unlock(&myMutex);

        // tell prod --> cond
        printf("CONSUMER: signaling myCond1\n");
        fflush(stdout);
        pthread_cond_signal(&myCond1);

        // error check on count
        if(myCount >= 10){
            break;
        }
    }

    return NULL;
}

/***********************************************
** main function which runs the program, creates
** the threads, joins, then prints end of prog
***********************************************/
int main(){

    printf("PROGRAM START\n\n");
    fflush(stdout);

    // create thread for consumer 
    printf("CONSUMER THREAD CREATED\n");
    fflush(stdout);
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // create thread for producer
    pthread_t producer_thread;
    pthread_create(&producer_thread, NULL, producer, NULL);

    // lets prod then con
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // prog finisheddd
    printf("\nEND OF PROGRAM\n");
    fflush(stdout);

    return 0;
}
