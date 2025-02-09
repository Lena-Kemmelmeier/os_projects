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

    // initialize variables
    int maxStringLength = 500; // arbitrary limit on the string
    int maxNumWords = 500; // arbitrary limit
    char cwd[maxStringLength]; // statically allocated
    char cli[maxStringLength]; // this is the input from user in CLI, statically allocated
    char *netID = "lkemmelmeier"; // my NetID, for the prompt

    int count = 0;

    for(;;){ // start of for loop, loop until the user chooses too - this is the example of an infinite loop from the class materials

        getcwd(cwd, maxStringLength); // gets the current working directory, this is stored in cwd

        // display the prompt to the user
        printf("%s:%s$", netID, cwd);

        // get input from the user
        fgets(cli, maxStringLength, stdin); // fgets allows us to take an input that has spaces in it
        //printf("%s",input); // check - are we getting the input correctly?

        // parse the input to the user
        char separatedInput[maxNumWords][maxStringLength]; // allocate 2d static arr that will store the parsed strings
        int numValidElements; // the valid number of cli elements
        numValidElements = parseInput(cli,separatedInput, maxStringLength); 
        // printf("num valid elements: %d\n",numValidElements); // check - correct number of valid elements

        // check - what do the contents of the 2d array look like?
        // for (int i = 0; i < numValidElements; i++){
        //     char* currentString = separatedInput[i];
        //     printf("current string: %s\n",currentString);
        // }

        // check the first element of the string array (first word)
        char* firstElement = separatedInput[0];
        // printf("first elements: %s\n",firstElement);
    
        // check if we should run the change dir function
        if((strcmp(firstElement,"cd") == 0) || strcmp(firstElement,"cd\n") == 0){

            // printf("num valid elements: %d\n",numValidElements); // check - correct number of valid elements

            // make sure that this command has two and only two inputs (i.e. cd and something else)
            if(numValidElements != 2){
                printf("Path Not Formatted Correctly!\n");
            }
            else{
                printf("run the cd command!\n"); // check
                char* path = separatedInput[1];

                printf("path: %s\n",path);
                changeDirectories("/home/lena/");

                int testy = strcmp(path,"/home/lena/");
                printf("test: %d",testy);
                
            }
        }


        count++;
        // put a stop to the for loop when necessary
        if (count > 4){
             break;
        }
        
    }
    
    return 0;
}

// function definitions

int parseInput(char* input, char splitWords[][500], int maxWords){
    char* word = strtok(input, " ");; // assumed delimeter is space here
    int wordCount = 0;

    // check - does strtok work?
    while(word != NULL && wordCount < maxWords){

        // I was having an issue with trailing new line chars in the last word/toke, so..
        int wordLength = strlen(word);

        if(word[wordLength - 1] == '\n'){
            word[wordLength - 1] = '\0'; // replace with null char
        }

        //printf("Word: %s\n", word); // check - are we parsing correctly?
        strcpy(splitWords[wordCount], word);
        wordCount++;
        word = strtok(NULL, " ");

    }

    return wordCount;

}

void changeDirectories(const char* path){
    int chDirResult = chdir(path);

    if(chDirResult == -1){
        printf("chdir Failed: %s\n", strerror(errno));
    }
}
