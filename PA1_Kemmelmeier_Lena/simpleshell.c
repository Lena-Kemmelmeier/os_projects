// Author: Lena Kemmelmeier
// Purpose: PA1: General process call API
// Date: Feb 8th 2025

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

// function prototypes
int parseInput(char* input, char splitWords[][500], int maxWords);
int executeCommand(char* const* enteredCommand, const char* infile, const char* outfile);
void changeDirectories(const char* path);

// main function

int main(){

    //for(;;){ // start of for loop, loop until the user chooses too - this is the example of an infinite loop from the class materials

        // initialize variables
        int maxStringLength = 100; // arbitrary limit
        char cwd[maxStringLength]; // statically allocated
        char *netID = "lkemmelmeier"; // my NetID, for the prompt
        getcwd(cwd, maxStringLength); 

        // display the prompt to the user
        printf("%s:%s$", netID, cwd);

        // get input from the user
        char input[maxStringLength];
        fgets(input, maxStringLength, stdin);
        //printf("%s",input); // check - are we getting the input correctly?

        // parse the input to the user
        
        
    //}
    
    return 0;
}

// function definitions