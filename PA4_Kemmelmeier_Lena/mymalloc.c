// Author: Lena Kemmelmeier
// Purpose: PA4: Dynamic Memory Allocator
// Date: April 8th 2025

// include libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

// struct declarations - provided in the assignment directions
typedef struct _mblock_t {
    struct _mblock_t * prev;
    struct _mblock_t * next;
    size_t size;
    int status;
    void * payload;
} mblock_t;

typedef struct _mlist_t {
    mblock_t *head;
} mlist_t;

// main function
int main(int argc, char* argv[]){
    return 0;
}

