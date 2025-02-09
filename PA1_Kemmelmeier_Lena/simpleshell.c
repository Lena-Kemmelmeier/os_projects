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
        int maxStringLength = 500; // arbitrary limit on the string
        char cwd[maxStringLength]; // statically allocated
        char cli[maxStringLength]; // this is the input from user in CLI, statically allocated
        char *netID = "lkemmelmeier"; // my NetID, for the prompt
        getcwd(cwd, maxStringLength); // gets the current working directory, this is stored in cwd

        // display the prompt to the user
        printf("%s:%s$", netID, cwd);

        // get input from the user
        fgets(cli, maxStringLength, stdin); // fgets allows us to take an input that has spaces in it
        //printf("%s",input); // check - are we getting the input correctly?

        // parse the input to the user
        char separatedInput[maxStringLength][500]; // allocate 2d static arr that will store the parsed strings
        int validElements; // the valid number of cli elements
        // here, 500 is the max number of strings/words (see function prototype we were given in the class materials)
        validElements = parseInput(cli,separatedInput, maxStringLength); 

        printf("num valid elements: %d\n",validElements);
        
    //}
    
    return 0;
}

// function definitions
int parseInput(char* input, char splitWords[][500], int maxWords){
    char* word = strtok(input, " ");; // assumed delimeter is space here
    int wordCount = 0;

    // check - does strtok work?
    while(word != NULL && wordCount < maxWords){
        printf("Word: %s\n", word);
        word = strtok(NULL, " ");
        //strcpy(splitWords[wordCount], word);
        wordCount++;
    }

    return wordCount;

}