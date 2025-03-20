// Author: Lena Kemmelmeier
// Purpose: PA3: Linux policy scheduler
// Date: March 15th 2025

// include libraries
#define _POSIX_C_SOURCE 200809L // had to use this instead of <sys/syscall.h> on my linux device - is this fine?
#define _GNU_SOURCE // had to use this instead of <sys/syscall.h> on my linux device - is this fine?

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h> // for the embdedded print_progress function
// #include <sys/syscall.h> // for the embdedded print_progress function
#include <sys/mman.h> // for the embdedded print_progress function
#include <linux/unistd.h> // had to use this instead of <sys/syscall.h> on my linux device - is this fine?

#define MAX_SIZE 2000000 // fixed array size

// these definitions are from the embdedded print_progress function
#define ANSI_COLOR_GRAY    "\x1b[30m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define TERM_CLEAR() printf("\033[H\033[J")
#define TERM_GOTOXY(x,y) printf("\033[%d;%dH", (y), (x))

// function prototypes
int readFile(char fileName[], int intArr[]);
void* arraySum(void* threadInputData);
void print_progress(pid_t localTid, size_t value); // this was the function given to us

// struct declaration
typedef struct _thread_data_t {
    int localTid;
    const int *data;
    int numVals;
    pthread_mutex_t *lock;
    long long int *totalSum;
} thread_data_t;

// main function
int main(int argc, char* argv[]){

    TERM_CLEAR(); // the provided print_progress.c file shows this called at the top of main

    // used dynamic memory allocation as we are expected to handle up to 100000000 numbers at once
    
    // check number of arguments, message if too few
    if (argc != 2){
        printf("Not enough parameters provided!\n");
        //printf("%d\n",argc);
        return -1;
    }

    // read in all data from the file that corresponds to the command-line-provided filename
    int* numbersArray = malloc(MAX_SIZE*sizeof(int));
    long long int totalSum = 0; // keeps track of the sum, as specified in the directions

    // initializing lock/mutex
    pthread_mutex_t lock; // created threads will use this for locking
    pthread_mutex_init(&lock, NULL); // NULL used in self-initializing routine example on die.net


    int numThreads = atoi(argv[1]); // argv[1] is the number of threads requested (second argument)
    //printf("%d\n",numThreads); // check

    // create the thread data array
    thread_data_t* threadDataArr = malloc(numThreads * sizeof(thread_data_t));
    for (int i = 0; i < numThreads; i++){
        threadDataArr[i].localTid = i;
        threadDataArr[i].data = numbersArray;
        threadDataArr[i].numVals = MAX_SIZE;
        threadDataArr[i].lock = &lock;
        threadDataArr[i].totalSum = &totalSum;

    }

    // create all the threads
    pthread_t* threads = malloc(numThreads * sizeof(pthread_t));
    for(int i = 0; i < numThreads; i++){
        pthread_create(&threads[i], NULL, arraySum, &threadDataArr[i]);
    }

    // join the threads
    for(int i = 0; i < numThreads; i++){
        pthread_join(threads[i], NULL); // wait for created threads to finish
    }

    // clean up!
    pthread_mutex_destroy(&lock); // destroy the lock
    free(numbersArray);
    free(threads);
    free(threadDataArr);

    return 0;
}

void* arraySum(void* threadInputData){
    thread_data_t *threadData = (thread_data_t*)threadInputData;
    long long int threadSum = 0;

    while(1){

        // initialize something that keeps track of the max latency
        long latency_max = 0; // I know this is a different style to how I have been naming variables - based on directions

        // let each thread sum over the whole length of the array
        for(int i = 0; i < threadData->numVals - 1; i++){
            struct timespec initalTime;
            clock_gettime(CLOCK_MONOTONIC, &initalTime); // since we are measuring elapsed time

            threadSum = threadSum + threadData->data[i];

            struct timespec endTime; // I chose to put this here because this directions say to create this at the endTime
            clock_gettime(CLOCK_MONOTONIC, &endTime); // since we are measuring elapsed time

            // calculate the time difference - treating the one billion in long int format
            long latency = ((endTime.tv_sec - initalTime.tv_sec) * 1000000000L) + (endTime.tv_nsec - initalTime.tv_nsec);

            // if this exceeds the max, update the max value
            if (latency > latency_max){
                latency_max = latency;
            }

        }

        // make it so only one thread at a time can update totalSum (shared between threads)
        pthread_mutex_lock(threadData->lock);
        *(threadData->totalSum) = *(threadData->totalSum) + threadSum; // this is the critical space!
        pthread_mutex_unlock(threadData->lock);

        // report latency max to the terminal
        print_progress(threadData->localTid, latency_max);

    }
    
    return NULL;
}

void print_progress(pid_t localTid, size_t value) {
    pid_t tid = syscall(__NR_gettid);
    TERM_GOTOXY(0,localTid+1);

	char prefix[256];
    size_t bound = 100;
    sprintf(prefix, "%d: %ld (ns) \t[", tid, value);
	const char suffix[] = "]";
	const size_t prefix_length = strlen(prefix);
	const size_t suffix_length = sizeof(suffix) - 1;
	char *buffer = calloc(bound + prefix_length + suffix_length + 1, 1);
	size_t i = 0;

	strcpy(buffer, prefix);
	for (; i < bound; ++i){
	    buffer[prefix_length + i] = i < value/10000 ? '#' : ' ';
	}
	strcpy(&buffer[prefix_length + i], suffix);
        
        if (!(localTid % 7)) 
            printf(ANSI_COLOR_WHITE "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 6)) 
            printf(ANSI_COLOR_BLUE "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 5)) 
            printf(ANSI_COLOR_RED "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 4)) 
            printf(ANSI_COLOR_GREEN "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 3)) 
            printf(ANSI_COLOR_CYAN "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 2)) 
            printf(ANSI_COLOR_YELLOW "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 1)) 
            printf(ANSI_COLOR_MAGENTA "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else
            printf("\b%c[2K\r%s\n", 27, buffer);

	fflush(stdout);
	free(buffer);
}