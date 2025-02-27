// Author: Lena Kemmelmeier
// Purpose: PA2: General threading API
// Date: Feb 24th 2025

// include libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h> // I understand that this is not stricly necessary here, but included it since I use pid_t (seemed like good practice)
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h> // I don't think this is super necessary for this assignment, but included it for mkdir just in case
#include <pthread.h> // for pthreads
#include <sys/time.h>

#define MAX_SIZE 100000000
//#define MAX_SIZE 1000000

// function prototypes
int readFile(char fileName[], int intArr[]); // I am dynamically allocating memory due to a past seg fault issue
void* arraySum(void* threadInputData);

// struct declaration
typedef struct _thread_data_t {
    const int *data;
    int startInd;
    int endInd;
    pthread_mutex_t *lock;
    long long int *totalSum;
} thread_data_t;

// main function
int main(int argc, char* argv[]){

    // used dynamic memory allocation as we are expected to handle up to 100000000 numbers at once
    
    // check number of arguments, message if too few
    if (argc != 3){
        printf("Not enough parameters!\n");
        //printf("%d\n",argc);
        return -1;
    }

    // read in all data from the file that corresponds to the command-line-provided filename
    //int maxAmountNums = 100000000; // max number of numbers - as specified in assignment note
    // int maxAmountNums = 1000000; // max number of numbers - as specified in assignment note
    int* numbersArray = malloc(MAX_SIZE*sizeof(int));
    //int* numbersArray;
    int numValuesRead = readFile(argv[1], numbersArray); //argv[1] is the second argument aka fileNmae
    //int numValuesRead = readFile(argv[1], numbersArray); //argv[1] is the second argument aka fileNmae


    // printf("%d\n",numValuesRead);

    // make sure number of threads requested is less than amount of values read
    // atoi converts argv[2] from a string to an int
    int numThreads = atoi(argv[2]); // argv[2] is the number of threads requested (third argument)
    //printf("%d\n",numThreads); // check

    if(numThreads > numValuesRead){ 
        printf("Too many threads requested!\n");
        return -1;
    }

    // //check - is the array actually storing the ints from the file?
    // for (int i = 0; i < numValuesRead; i++){
    //      printf("%d\n",numbersArray[i]);
    // }

    long long int totalSum = 0; // keeps track of the sum, as specified in the directions

    // get the start time
    struct timeval initialTime, endTime; // chose to initialize both at once according to the StackOverflow example
    double elapsedTime; // this implementation is from the StackOverflow resource posted in the homework
    gettimeofday(&initialTime, NULL); // according to the die.net page, timezone argument should normally be NULL

    // initializing lock/mutex
    pthread_mutex_t lock; // created threads will use this for locking
    pthread_mutex_init(&lock, NULL); // NULL used in self-initializing routine example on die.net

    thread_data_t* threadDataArr = malloc(numThreads * sizeof(thread_data_t));
    //thread_data_t threadDataArr[numThreads];

    for (int i = 0; i < numThreads; i++){
        threadDataArr[i].data = numbersArray;
        threadDataArr[i].totalSum = &totalSum;
        threadDataArr[i].lock = &lock;
        
        // evening splitting of what this thread should sum through
        threadDataArr[i].startInd = (i * numValuesRead) / numThreads; // 0 times anything is 0
        threadDataArr[i].endInd = ((i + 1) * numValuesRead) / numThreads;
    }

    // create all the threads
    pthread_t* threads = malloc(numThreads * sizeof(pthread_t));
    //pthread_t threads[numThreads];
    for(int i = 0; i < numThreads; i++){
        pthread_create(&threads[i], NULL, arraySum, &threadDataArr[i]);
    }

    // join the threads
    for(int i = 0; i < numThreads; i++){
        pthread_join(threads[i], NULL); // wait for created threads to finish
    }

    // get the end time (this stops the timer)
    gettimeofday(&endTime, NULL); // according to the die.net page, timezone argument should normally be NULL

    // calculate the total execution time
    // this implementation is from the StackOverflow resource posted in the homework
    elapsedTime = (endTime.tv_sec - initialTime.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (endTime.tv_usec - initialTime.tv_usec) / 1000.0;   // us to ms
    printf("Total execution time: %f ms.\n", elapsedTime);
    printf("Final sum: %lld\n", totalSum);

    pthread_mutex_destroy(&lock); // destroy the lock
    free(numbersArray);
    free(threads);
    free(threadDataArr);
    return 0;
}

// function definitions
int readFile(char fileName[], int intArr[]){
    int numItemsParsed = 0; // number of numbers in the file

    // create input file stream and open it for reading
    FILE* file = fopen(fileName,"r");

    if(file == NULL){ // file does not exist
        printf("File not found...\n");
        return -1;
    }
    
    // the file exists, so read in the numbers
    // as long as we are reading in at least one int, store into array, increment count
    while(fscanf(file, "%d", &intArr[numItemsParsed]) == 1){
        numItemsParsed++;
    }

    fclose(file);
    return numItemsParsed;
}

void* arraySum(void* threadInputData){
    thread_data_t *threadData = (thread_data_t*)threadInputData;
    long long int threadSum = 0;

    // let each thread sum
    for(int i = threadData->startInd; i < threadData->endInd; i++){
        threadSum = threadSum + threadData->data[i];
    }

    // make it so only one thread at a time can update totalSum (shared between threads
    // avoid race conditions
    pthread_mutex_lock(threadData->lock);
    *(threadData->totalSum) = *(threadData->totalSum) + threadSum;
    pthread_mutex_unlock(threadData->lock);

    return NULL;
}