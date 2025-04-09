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

// struct declarations - provided in the assignment directions
typedef struct _mblock_t {
    struct _mblock_t* prev;
    struct _mblock_t* next;
    size_t size;
    int status;
    void* payload;
} mblock_t;

typedef struct _mlist_t {
    mblock_t* head;
} mlist_t;

#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload) // macro defnition, in the assignment directions

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
    printMemList(mlist.head);

    void * p1 = mymalloc(10);
    printMemList(mlist.head);

    void * p2 = mymalloc(100);
    printMemList(mlist.head);

    void * p3 = mymalloc(200);
    printMemList(mlist.head);
    
    void * p4 = mymalloc(500);
    printMemList(mlist.head);

    myfree(p3); p3 = NULL;
    printMemList(mlist.head);

    myfree(p2); p2 = NULL;
    printMemList(mlist.head);
    
    void * p5 = mymalloc(150);
    printMemList(mlist.head);

    void * p6 = mymalloc(500);
    printMemList(mlist.head);

    myfree(p4); p4 = NULL;
    printMemList(mlist.head);

    myfree(p5); p5 = NULL;
    printMemList(mlist.head);

    myfree(p6); p6 = NULL;
    printMemList(mlist.head);

    myfree(p1); p1 = NULL;
    printMemList(mlist.head);

    return 0;
}

// function definitions - required functions
void* mymalloc(size_t size){

    // dummy check - if we don't need won't to allocate memory, don't
    if(size == 0){
        return NULL;
    } 

    // look for first block that can accomodate the size of what we need
    mblock_t *block = findFreeBlockOfSize(size);

    if(block != NULL){ // if we found a free block that fits the request

        // is the free block big enough to split?
        if(block->size > size + sizeof(mblock_t)){
            splitBlockAtSize(block, size);
        }

        block->status = 1; // this block is no longer free
        return block->payload;
    }

    // if block is null, we get here. this means we failed to find a block that can accomodate our request
    block = growHeapBySize(size); // grow the heap in size!
    if(block == NULL){
        return NULL;
    } 

    block->status = 1;
    return block->payload; // newly reserved memory blocks's VA location
}

void myfree(void* ptr){

    if(ptr == NULL){
        return;
    }

    void *header = (char*)ptr - MBLOCK_HEADER_SZ;
    mblock_t* block = (mblock_t*)header;

    if((void*)block < (void*)mlist.head || (void*)block >= sbrk(0)){
        return;
    }

    block->status = 0;
    coallesceBlockNext(block);
    coallesceBlockPrev(block);
}

// function definitions - additional functions
mblock_t* findLastMemlistBlock(void){

    // start at whatever memory block is at the head
    mblock_t* currentBlock = mlist.head;

    // if the list is empty, return
    if(currentBlock == NULL){
        return NULL;
    }
    
    // otherwise, traverse across memory list until we find last block
    while(currentBlock->next != NULL){
        currentBlock = currentBlock->next;
    }

    return currentBlock;
}

// 
mblock_t* findFreeBlockOfSize(size_t size) {

    // current block is the head of the list
    mblock_t *currentBlock = mlist.head;

    while(currentBlock != NULL) {

        // as soon as we find a free block that is of minimum size to what we need, exit function
        if(currentBlock->status == 0 && currentBlock->size >= size){
            return currentBlock;
        }

        // otherwise, look at next block in the memory list
        currentBlock = currentBlock->next;
    }

    // if we didn't find any free block that could accomodate the request, return NULL
    return NULL;
}

void splitBlockAtSize(mblock_t* block, size_t newSize){

    size_t totalSize = block->size;

    if(totalSize < newSize + sizeof(mblock_t) + 1){
        return; // not enough to split
    } 

    // calculate the end of the allocated block
    char* newBlockAddr = + newSize + (char*)block + MBLOCK_HEADER_SZ;
    mblock_t* newBlock = (mblock_t*)newBlockAddr; // reinterpret the address

    // 
    newBlock->size = totalSize - newSize - MBLOCK_HEADER_SZ;
    newBlock->status = 0;
    newBlock->prev = block;
    newBlock->next = block->next;
    newBlock->payload = (char*)newBlock + MBLOCK_HEADER_SZ;

    if(block->next != NULL){
        block->next->prev = newBlock;
    }
    block->next = newBlock;
    block->size = newSize;
}

void coallesceBlockPrev(mblock_t* freedBlock){

    mblock_t* prev = freedBlock->prev;

    if(prev != NULL && !prev->status){
        prev->size += MBLOCK_HEADER_SZ + freedBlock->size;
        prev->next = freedBlock->next;

        if (freedBlock->next != NULL){
            freedBlock->next->prev = prev;
        } 
    }
}

void coallesceBlockNext(mblock_t* freedBlock){

    // extra check...
    if(freedBlock == NULL){
        return;
    }

    mblock_t* nextBlock = freedBlock->next;

    if(nextBlock != NULL && nextBlock->status == 0){
        freedBlock->size += MBLOCK_HEADER_SZ + nextBlock->size;
        freedBlock->next = nextBlock->next;

        if(nextBlock->next != NULL){
            nextBlock->next->prev = freedBlock;
        }
    }
}

mblock_t* growHeapBySize(size_t size){

    // need to grow the heap by a minimum of 1 KB - prevent fragmentation
    size_t growSize = MBLOCK_HEADER_SZ + size;
    if(growSize < 1024){
        growSize = 1024;    
    } 

    // this lil chunk was in the lecture PPT
    // request more heap memory!
    void* mem = sbrk(growSize);
    if(mem == (void*)-1){
        return NULL; // could not extend heap
    }

    // initialize a new memory block after grown
    mblock_t* block = (mblock_t*)mem;
    block->prev = NULL;
    block->status = 0;
    block->next = NULL;
    block->size = growSize - MBLOCK_HEADER_SZ; // usable space of block (exclude header)
    block->payload = (char*)block + MBLOCK_HEADER_SZ; // point to start of usable space right after the header

    // if the memory list is empty, this new block will be the head
    if(mlist.head == NULL){
        mlist.head = block;
        return block;
    }

    // if we already have other memory blocks in the memory list, add this to the end of the memory list
    mblock_t* last = findLastMemlistBlock();
    block->prev = last;
    last->next = block;
    return block;

}

void printMemList(const mblock_t* head){

  const mblock_t* p = head;
  size_t i = 0;

  while(p != NULL){
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