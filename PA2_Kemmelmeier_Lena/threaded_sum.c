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
#include <pthread.h> // for pthreds
#include <sys/time.h>

// function prototypes
int readFile(char fileName[], int intArr[]);

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
    
    if (argc != 3){
        printf("Not enough parameters!\n");
        printf("%d\n",argc);
    }
    else{

    }

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
    else{ // the file exists, read in the numbers


        // as long as we are reading in at least one int, store into array, increment count
        while(fscanf(file, "%d", &intArr[numItemsParsed]) > 0){
            numItemsParsed++;
        }

        return numItemsParsed;
    }
}
