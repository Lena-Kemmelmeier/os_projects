// Author: Lena Kemmelmeier
// Purpose: PA4: Dynamic Memory Allocator
// Date: April 10th 2025

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
    printMemList(mlist.head); // when we print, we should just see just ===========

    void* p1 = mymalloc(10);
    // mymalloc should not find a free block, it will call growHeapBySize(10), which will allocate a chunk of at least 1 KB
    printMemList(mlist.head); // header 32 bytes

    void* p2 = mymalloc(100);
    // now we should see two allocated blocks (needed to allocate another 1 KB chunk)
    printMemList(mlist.head);

    void* p3 = mymalloc(200);
    // grow the heap again
    printMemList(mlist.head);
    
    void* p4 = mymalloc(500);
    // . . . and again!
    printMemList(mlist.head);

    myfree(p3); p3 = NULL;
    // this should free p3 - which shows up as block 2 in the printMemList output (starts at block 0)
    printMemList(mlist.head);

    myfree(p2); p2 = NULL;
    // this makes it so two blocks are both adjacent and free
    // this means the deallocator coalesces the two free blocks, merging them
    // merged block is of size 2016 = 992 + 992 + 32 (this is the header)
    printMemList(mlist.head);
    
    void* p5 = mymalloc(150);
    // searches for a block of at least 150 bytes, finds this big free block of 2016 bytes
    // there's enough space to split, so split block and remaining space is for a block added to the block list (size 1834)
    printMemList(mlist.head);

    void* p6 = mymalloc(500);
    // searches for a block at least 500 bytes, finds the block that is 1834 bytes
    // there's enough space to split, so split block and remainign space is fot a block added to the block list (size 1302)
    printMemList(mlist.head);

    myfree(p4); p4 = NULL;
    // free p3 which had been the last block of 992 bytes
    // this means we have two blocks that are adjacent and free, so they coalesce into a mega block of 2326 bytes
    printMemList(mlist.head);

    myfree(p5); p5 = NULL;
    // this frees the block that is 150 bytes, no coalescing because it is not adjacent to another free block
    printMemList(mlist.head);

    myfree(p6); p6 = NULL;
    // now, we free the block that is 500 bytes
    // this made it so the block that is 2336 bytes, this newly free block, and the 150 bytes block are all coalesced together
    // results in a mega block that is 3040 bytes big
    printMemList(mlist.head);

    myfree(p1); p1 = NULL;
    // finally, free the last allocated block (we coalesce with the adjacent free block), results in one big free block that is 4064 bytes
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

    return; // not necessary, I know
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
        return; // not enough to split!
    } 

    // calculate the end of the allocated block
    char* newBlockAddr = (char*)block + newSize + MBLOCK_HEADER_SZ; // starting address of the new block that will be created when splitting larger block
    // we skip over the header and the amount of data we're allocating


    mblock_t* newBlock = (mblock_t*)newBlockAddr; // reinterpret the address

    // how much memory do we have left after taking out the portion for the allocated block?
    newBlock->size = totalSize - newSize - MBLOCK_HEADER_SZ; // original block size - how much we're allocating - header size
    newBlock->status = 0;

    // set the links
    newBlock->prev = block;
    newBlock->next = block->next;

    newBlock->payload = (char*)newBlock + MBLOCK_HEADER_SZ; // skip over the header

    mblock_t* nextBlock = block->next;

    // if there is a block after
    if(nextBlock != NULL){
        nextBlock->prev = newBlock; // set the previous of this next block to point back at this new block
    }

    block->next = newBlock; // the next block points next to the new block that was just created
    block->size = newSize; // match the requested allocation size

    return; // not necessary, I know

}

void coallesceBlockPrev(mblock_t* freedBlock){

    mblock_t* prevBlock = freedBlock->prev;

    // if there is a previous block and it is free
    if(prevBlock != NULL && prevBlock->status == 0){
        prevBlock->size = prevBlock->size + MBLOCK_HEADER_SZ + freedBlock->size; // add the side of the freed block to this previous block
        prevBlock->next = freedBlock->next; // the next of the previous block will point to the the next of the freed block


        mblock_t* nextBlock = freedBlock->next;

        // if there's a block after this freed block, make sure that block points back to the previous block
        if (nextBlock != NULL){
            nextBlock->prev = prevBlock;
        } 
    }

    return; // not necessary, I know
}

void coallesceBlockNext(mblock_t* freedBlock){

    // extra check...
    if(freedBlock == NULL){
        return;
    }

    mblock_t* nextBlock = freedBlock->next;

    // if there is a next block and it is free
    if(nextBlock != NULL && nextBlock->status == 0){
        freedBlock->size = freedBlock->size + MBLOCK_HEADER_SZ + nextBlock->size; // adds the size of the next block to this freed block
        freedBlock->next = nextBlock->next; // skip next block = the next of freed block will point to the block after the next block

        mblock_t* nextNextBlock = nextBlock->next;

        // if there's a block after the next block, we need to update its previous ptr so that it points back to the freed block now
        if(nextNextBlock != NULL){
            nextNextBlock->prev = freedBlock;
        }
    }

    return; // not necessary, I know
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

    // initialize a new memory block after growing
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

    // set the links
    block->prev = last;
    last->next = block;

    return block;

}

// supplied from the assignment
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
