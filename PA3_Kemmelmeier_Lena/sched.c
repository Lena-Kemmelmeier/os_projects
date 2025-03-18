// Author: Lena Kemmelmeier
// Purpose: PA3: Linux policy scheduler
// Date: March 15th 2025

// include libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h> // for pthreads
#include <time.h> // for clock_gettime

#define MAX_SIZE 2000000 // fixed array size

// function prototypes
int readFile(char fileName[], int intArr[]);
void* arraySum(void* threadInputData);

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



    printf("Final sum: %lld\n", totalSum);

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
        // let each thread sum over the whole length of the array
        for(int i = 0; i < threadData->numVals - 1; i++){
            struct timespec start;
            clock_gettime(NULL, &start);

            threadSum = threadSum + threadData->data[i];

            struct timespec end = clock_gettime();
        }

        // make it so only one thread at a time can update totalSum (shared between threads)
        pthread_mutex_lock(threadData->lock);
        *(threadData->totalSum) = *(threadData->totalSum) + threadSum; // this is the critical space!
        pthread_mutex_unlock(threadData->lock);
    }
    

    return NULL;
}