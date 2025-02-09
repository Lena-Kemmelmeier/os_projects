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



    return 0;
}