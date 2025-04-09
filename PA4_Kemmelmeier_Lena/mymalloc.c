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

#include <pthread.h> // this was included in the print_memlist.c file
#include <ctype.h> // this was included in the print_memlist.c file
#include <time.h> // this was included in the print_memlist.c file
#include <sys/syscall.h> // this was included in the print_memlist.c file
#include <sys/mman.h> // this was included in the print_memlist.c file

#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload) // macro defnition, in the assignment directions

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

// function prototypes - required functions
void* mymalloc(size_t size);
void myfree(void* ptr);

// function prototypes - additional functions
mblock_t* findLastMemlistBlock(void);
mblock_t* findFreeBlockOfSize(size_t size);
void splitBlockAtSize(mblock_t * block, size_t newSize);
void coallesceBlockPrev(mblock_t* freedBlock);
void coallesceBlockNext(mblock_t* freedBlock);
mblock_t* growHeapBySize(size_t size);
void printMemList(const mblock_t* head);

// global heap memory list
mlist_t mlist;

// main function
int main(int argc, char* argv[]){

    mlist.head = NULL; // initialize head to NULL

    // void * p1 = mymalloc(10);
    // void * p2 = mymalloc(100);
    // void * p3 = mymalloc(200);
    // void * p4 = mymalloc(500);
    // myfree(p3); p3 = NULL;
    // myfree(p2); p2 = NULL;
    // void * p5 = mymalloc(150);
    // void * p6 = mymalloc(500);
    // myfree(p4); p4 = NULL;
    // myfree(p5); p5 = NULL;
    // myfree(p6); p6 = NULL;
    // myfree(p1); p1 = NULL;

    return 0;
}

// function definitions - required functions
void* mymalloc(size_t size){
    if (size == 0) return NULL;

    mblock_t *block = findFreeBlockOfSize(size);
    if (block) {
        if (block->size > size + sizeof(mblock_t)) {
            splitBlockAtSize(block, size);
        }
        block->status = 1;
        return block->payload;
    }

    block = growHeapBySize(size);
    if (!block) return NULL;

    block->status = 1;
    return block->payload;
}

void myfree(void* ptr){
    if (!ptr) return;

    void *header = (char *)ptr - MBLOCK_HEADER_SZ;
    mblock_t *block = (mblock_t *)header;

    if ((void *)block < (void *)mlist.head || (void *)block >= sbrk(0)) return;

    block->status = 0;
    coallesceBlockNext(block);
    coallesceBlockPrev(block);
}

// function definiitons - additional functions
mblock_t* findLastMemlistBlock(void){
    mblock_t *curr = mlist.head;
    if (!curr) return NULL;
    while (curr->next) curr = curr->next;
    return curr;
}

mblock_t* findFreeBlockOfSize(size_t size) {
    mblock_t *curr = mlist.head;
    while (curr) {
        if (!curr->status && curr->size >= size) return curr;
        curr = curr->next;
    }
    return NULL;
}

void splitBlockAtSize(mblock_t * block, size_t newSize){
    size_t totalSize = block->size;
    if (totalSize < newSize + sizeof(mblock_t) + 1) return; // not enough to split

    char *newBlockAddr = (char *)block + MBLOCK_HEADER_SZ + newSize;
    mblock_t *newBlock = (mblock_t *)newBlockAddr;

    newBlock->size = totalSize - newSize - MBLOCK_HEADER_SZ;
    newBlock->status = 0;
    newBlock->prev = block;
    newBlock->next = block->next;
    newBlock->payload = (char *)newBlock + MBLOCK_HEADER_SZ;

    if (block->next) block->next->prev = newBlock;
    block->next = newBlock;
    block->size = newSize;
}

void coallesceBlockPrev(mblock_t * freedBlock){
    mblock_t *prev = freedBlock->prev;
    if (prev && !prev->status) {
        prev->size += MBLOCK_HEADER_SZ + freedBlock->size;
        prev->next = freedBlock->next;
        if (freedBlock->next) freedBlock->next->prev = prev;
    }
}

void coallesceBlockNext(mblock_t* freedBlock){
    mblock_t *next = freedBlock->next;
    if (next && !next->status) {
        freedBlock->size += MBLOCK_HEADER_SZ + next->size;
        freedBlock->next = next->next;
        if (next->next) next->next->prev = freedBlock;
    }
}

mblock_t * growHeapBySize(size_t size){
    size_t growSize = size + MBLOCK_HEADER_SZ;
    if (growSize < 1024){
        growSize = 1024;    
    } 

    void *mem = sbrk(growSize);
    if (mem == (void *)-1) return NULL;

    mblock_t *newBlock = (mblock_t *)mem;
    newBlock->size = growSize - MBLOCK_HEADER_SZ;
    newBlock->status = 0;
    newBlock->prev = NULL;
    newBlock->next = NULL;
    newBlock->payload = (char *)newBlock + MBLOCK_HEADER_SZ;

    if (!mlist.head) {
        mlist.head = newBlock;
    } else {
        mblock_t *last = findLastMemlistBlock();
        last->next = newBlock;
        newBlock->prev = last;
    }

    return newBlock;
}

void printMemList(const mblock_t* head){
  const mblock_t* p = head;
  
  size_t i = 0;
  while(p != NULL) {
    printf("[%ld] p: %p\n", i, (void*)p);
    printf("[%ld] p->size: %ld\n", i, p->size);
    printf("[%ld] p->status: %s\n", i, p->status > 0 ? "allocated" : "free");
    printf("[%ld] p->prev: %p\n", i, (void*)p->prev);
    printf("[%ld] p->next: %p\n", i, (void*)p->next);
    printf("___________________________\n");
    ++i;
    p = p->next;
  }
  printf("===========================\n");
}



