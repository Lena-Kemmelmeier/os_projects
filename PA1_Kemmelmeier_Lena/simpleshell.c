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
    int numValidElements; // the valid number of cli elements

    for(;;){ // start of for loop, loop until the user chooses too - this is the example of an infinite loop from the class materials

        getcwd(cwd, maxStringLength); // gets the current working directory, this is stored in cwd
        printf("%s:%s$ ", netID, cwd); // display the prompt to the user


        // get input from the user
        fgets(cli, maxStringLength, stdin); // fgets allows us to take an input that has spaces in it
        //printf("%s",input); // check - are we getting the input correctly?

        // parse the input to the user
        char separatedInput[maxNumWords][maxStringLength]; // allocate 2d static arr that will store the parsed strings
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
        if(strcmp(firstElement, "cd") == 0){

            // printf("num valid elements: %d\n",numValidElements); // check - correct number of valid elements

            // make sure that this command has two and only two inputs (i.e. cd and something else)
            if(numValidElements != 2){
                printf("Path Not Formatted Correctly!\n");
            }
            else{
                // printf("run the cd command!\n"); // check
                char* path = separatedInput[1];

                // printf("path: %s\n",path); // check
                changeDirectories(path);
                
            }
        }
        else if(strcmp(firstElement, "exit") == 0){
            return 0;
        }
        else{ // this is a command/executable that would be executed in a child process

            char* infile = NULL;
            char* outfile = NULL;
            int commandCtr = 0;
            char** commandArr = (char**)malloc((numValidElements + 1) * sizeof(char*));;
            _Bool justFoundRedirChar = 0;

            // loop over, check for redirection characters
            for(int i = 0; i < numValidElements; i++){
                char* currentWord = separatedInput[i];
                
                if(strcmp(currentWord, ">") == 0){ // check for output redirection char
                    //printf("Output redirection character found at %d\n",i); // check
                    outfile = separatedInput[i + 1]; // the char after > is the outfile
                    //printf("Outfile: %s\n",outfile); // check
                    justFoundRedirChar = 1;
                }
                else if(strcmp(currentWord, "<") == 0){ // check for input redirection char
                    infile = separatedInput[i + 1];
                    //printf("Infile: %s\n",infile); // check
                    justFoundRedirChar = 1;
                }
                else if(justFoundRedirChar == 0){ // make sure this word is not an infile/outfile string
                    commandArr[i] = (char*)malloc(strlen(separatedInput[i]) + 1); // the +1 allows us to get the /0
                    strcpy(commandArr[i], separatedInput[i]);

                    commandCtr++;
                    justFoundRedirChar = 0;
                }

            }
            
            // last char in array should be a NULL character
            commandArr[commandCtr] = NULL;
            
            // print the command array - check
            //for(int i = 0; i < commandCtr + 1; i++){
            //     printf("Current command: %s\n",commandArr[i]);
            //}

            executeCommand(commandArr, infile, outfile);

            // deallocate commandArr
            for (int i = 0; i < commandCtr + 1; i++) {
                free(commandArr[i]); // free each string
            }
            free(commandArr); // now free the arr of pointers

        }

    }
    
    return 0;
}

// function definitions

int parseInput(char* input, char splitWords[][500], int maxWords){
    char* word = strtok(input, " ");; // assumed delimeter is space here
    int wordCount = 0;

    // check - does strtok work?
    while(wordCount < maxWords && word != NULL){

        // I was having an issue with trailing new line chars in the last word/toke, so..
        int wordLength = strlen(word);

        if(word[wordLength - 1] == '\n'){ //if the last char in the word/toke is a newline..
            word[wordLength - 1] = '\0'; // replace with null char (was messing up things like chdir otherwise)
        }

        //printf("Word: %s\n", word); // check - are we parsing correctly?
        strcpy(splitWords[wordCount], word);
        word = strtok(NULL, " ");
        wordCount++;

    }

    return wordCount;

}

void changeDirectories(const char* path){
    int chDirResult = chdir(path);

    if(chDirResult == -1){
        printf("chdir Failed: %s\n", strerror(errno));
    }
}

int executeCommand(char* const* enteredCommand, const char* infile, const char* outfile){
    int result = -1; // result = -1 indicates a failure, result = 0 will be a success
    // I used the three easy principles textbook (ostep) examples from chapter 5 as a guide 

    // fork - create a child process from parent process
    pid_t pid = fork();

    if(pid == -1){ // fork failed
        printf("fork Failed: %s\n", strerror(errno));
        return result;
    }

    if(pid == 0){ // fork was successful, we are in child process

        // allowed to assume only one output/input redirection at a time
        if(outfile != NULL){ // output redirection
            int fileDescriptor = open(outfile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
            dup2(fileDescriptor, STDOUT_FILENO); //STDOUT_FILENO is 1

            if(fileDescriptor == -1){
                fprintf(stderr, "Error opening input file: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else{
                dup2(fileDescriptor, STDIN_FILENO); //STDIN_FILENO is 0
                close(fileDescriptor);
            }

        }
        else if(infile != NULL){ // input redirection
            int fileDescriptor = open(infile, O_RDONLY, 0666);

            if(fileDescriptor == -1){
                fprintf(stderr, "Error opening input file: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else{
                dup2(fileDescriptor, STDIN_FILENO); //STDIN_FILENO is 0
                close(fileDescriptor);
            }
        }
        if(execvp(enteredCommand[0], enteredCommand) == -1) {
            fprintf(stderr, "exec Failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else{ // we are in parent process - wait for child to finish
        int waitResult;
        wait(&waitResult);

        if (WIFEXITED(waitResult)) {
            printf("Child finished with error status: %d\n", WEXITSTATUS(waitResult));
            return result;
        }

    }

    
    return result;
}