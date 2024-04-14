//xvasak01

/**
 * Implementace My MALloc
 * Demonstracni priklad pro 1. ukol IPS/2022
 * Ales Smrcka
 *
 * Vypracovala: Katerina Cepelkova, xcepel03, FIT
 */

#include "mmal.h"
#include <sys/mman.h> // mmap
#include <stdbool.h> // bool
#include <assert.h> // assert
#include <string.h> // memcpy
#define FIRST_HDR_PTR (Header *)((char *)first_arena + sizeof(Arena))

#ifdef NDEBUG
/**
 * The structure header encapsulates data of a single memory block.
 *   ---+------+----------------------------+---
 *      |Header|DDD not_free DDDDD...free...|
 *   ---+------+-----------------+----------+---
 *             |-- Header.asize -|
 *             |-- Header.size -------------|
 */
typedef struct header Header;
struct header {

    /**
     * Pointer to the next header. Cyclic list. If there is no other block,
     * points to itself.
     */
    Header *next;

    /// size of the block
    size_t size;

    /**
     * Size of block in bytes allocated for program. asize=0 means the block 
     * is not used by a program.
     */
    size_t asize;
};

/**
 * The arena structure.
 *   /--- arena metadata
 *   |     /---- header of the first block
 *   v     v
 *   +-----+------+-----------------------------+
 *   |Arena|Header|.............................|
 *   +-----+------+-----------------------------+
 *
 *   |--------------- Arena.size ---------------|
 */
typedef struct arena Arena;
struct arena {

    /**
     * Pointer to the next arena. Single-linked list.
     */
    Arena *next;

    /// Arena size.
    size_t size;
};

#define PAGE_SIZE (128*1024)

#endif // NDEBUG

Arena *first_arena = NULL;
// &(first_arena[1]) = adresa prvniho headeru

/**
 * Return size alligned to PAGE_SIZE
 */
static
size_t allign_page(size_t size)
{
    if (size % PAGE_SIZE == 0) {
        return PAGE_SIZE * (size / PAGE_SIZE);
    } 
    return PAGE_SIZE * (size / PAGE_SIZE + 1);
}

/**
 * Allocate a new arena using mmap.
 * @param req_size requested size in bytes. Should be alligned to PAGE_SIZE.
 * @return pointer to a new arena, if successfull. NULL if error.
 * @pre req_size > sizeof(Arena) + sizeof(Header)
 */

/**
 *   +-----+------------------------------------+
 *   |Arena|....................................|
 *   +-----+------------------------------------+
 *
 *   |--------------- Arena.size ---------------|
 */
static
Arena *arena_alloc(size_t req_size)
{
    assert(req_size > sizeof(Arena) + sizeof(Header));
    Arena *new_arena = mmap(NULL, allign_page(req_size), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0); //MAP_ANON must have -1
    if (new_arena == MAP_FAILED) {
        return NULL;
    }
    
    new_arena->size = allign_page(req_size); //?

    return new_arena;
}

/**
 * Appends a new arena to the end of the arena list.
 * @param a     already allocated arena
 */
static
void arena_append(Arena *a)
{
    if (!first_arena) {
        first_arena = a;
    } else {
        Arena *tmp = first_arena; 
        while (tmp->next) {
            tmp = tmp->next;
        }
        tmp->next = a;
    }
}

/**
 * Header structure constructor (alone, not used block).
 * @param hdr       pointer to block metadata.
 * @param size      size of free block
 * @pre size > 0
 */
/**
 *   +-----+------+------------------------+----+
 *   | ... |Header|........................| ...|
 *   +-----+------+------------------------+----+
 *
 *                |-- Header.size ---------|
 */
static
void hdr_ctor(Header *hdr, size_t size)
{
    assert(size > 0);
    hdr->next = NULL; //next header
    hdr->size = size; //size of block (no header)
    hdr->asize = 0; //allocated space
}

/**
 * Checks if the given free block should be split in two separate blocks.
 * @param hdr       header of the free block
 * @param size      requested size of data
 * @return true if the block should be split
 * @pre hdr->asize == 0
 * @pre size > 0
 */
static
bool hdr_should_split(Header *hdr, size_t size)
{
    assert(size > 0);
    assert(hdr->asize == 0);
    return (hdr->size > size + sizeof(Header)); //true = (>= H + size + 1B)
}

/**
 * Splits one block in two.
 * @param hdr       pointer to header of the big block
 * @param req_size  requested size of data in the (left) block.
 * @return pointer to the new (right) block header.
 * @pre   (hdr->size >= req_size + 2*sizeof(Header))
 */
/**
 * Before:        |---- hdr->size ---------|
 *
 *    -----+------+------------------------+----
 *         |Header|........................|
 *    -----+------+------------------------+----
 *            \----hdr->next---------------^
 */
/**
 * After:         |- req_size -|
 *
 *    -----+------+------------+------+----+----
 *     ... |Header|............|Header|....|
 *    -----+------+------------+------+----+----
 *             \---next--------^  \--next--^
 */
static
Header *hdr_split(Header *hdr, size_t req_size)
{
    assert(hdr->size >= req_size + 2*sizeof(Header));
    char *tmp = (char *)hdr;
    Header *hdr_new = (Header *)(tmp + sizeof(Header) + req_size); //pointer to one
    hdr_ctor(hdr_new, hdr->size - (sizeof(Header) + req_size));
   
    hdr_new->next = hdr->next;        
    hdr->next = hdr_new;
    hdr->asize = req_size;
    hdr->size = req_size;
    
    return hdr_new;
}

/**
 * Detect if two adjacent blocks could be merged.
 * @param left      left block
 * @param right     right block
 * @return true if two block are free and adjacent in the same arena.
 * @pre left->next == right
 * @pre left != right
 */
static
bool hdr_can_merge(Header *left, Header *right)
{
    assert(left->next == right);
    assert(left!= right);
    if (left->asize == 0 && right->asize == 0){
        char *tmp = (char *)left;
        if ((Header *)(tmp + sizeof(Header) + left->size) == right)
            return true;
    }
    return false;
}

/**
 * Merge two adjacent free blocks.
 * @param left      left block
 * @param right     right block
 * @pre left->next == right
 * @pre left != right
 */
static
void hdr_merge(Header *left, Header *right)
{
    assert(left->next == right);
    assert(left!= right);
    left->size += right->size + sizeof(Header);
    left->next = right->next;
}

/**
 * Finds the free block that fits best to the requested size.
 * @param size      requested size
 * @return pointer to the header of the block or NULL if no block is available.
 * @pre size > 0
 */
static
Header *best_fit(size_t size)
{  
    assert(size > 0);
    Header *best = NULL;   
    if (first_arena != NULL) {
        Header *hdr_iter = FIRST_HDR_PTR;
        do { 
            if (hdr_iter->asize == 0 && hdr_iter->size >= size) {
                if (best == NULL || hdr_iter->size < best->size) {
                    best = hdr_iter;
                }
            }
            hdr_iter = hdr_iter->next;
        } while (hdr_iter != FIRST_HDR_PTR);
    }
    return best;
}

/**
 * Search the header which is the predecessor to the hdr. Note that if 
 * @param hdr       successor of the search header
 * @return pointer to predecessor, hdr if there is just one header.
 * @pre first_arena != NULL
 * @post predecessor->next == hdr
 */
static
Header *hdr_get_prev(Header *hdr)
{
    assert(first_arena != NULL);
    Header *hdr_iter = FIRST_HDR_PTR;
    do { 
        if (hdr_iter->next == hdr) {
            return hdr_iter;
        }
        hdr_iter = hdr_iter->next;
    } while (hdr_iter != FIRST_HDR_PTR);
    return NULL;
}

/**
 * Allocate memory. Use best-fit search of available block.
 * @param size      requested size for program
 * @return pointer to allocated data or NULL if error or size = 0.
 */
void *mmalloc(size_t size) 
{
    Header *new_h = best_fit(size);
    if (!new_h) {
        Arena *new_a = arena_alloc(allign_page(size + sizeof(Arena) + sizeof(Header)));
        if (!new_a)
            return NULL;
        arena_append(new_a);
        
        char *tmp = (char *)new_a;
        new_h = (Header *)(tmp + sizeof(Arena));
        hdr_ctor(new_h, allign_page(size + sizeof(Arena) + sizeof(Header)) - sizeof(Arena) - sizeof(Header));
        new_h->next = FIRST_HDR_PTR;
        
        Header *hdr_iter = FIRST_HDR_PTR;
        while (hdr_iter->next != FIRST_HDR_PTR) {
            hdr_iter = hdr_iter->next;
        }
        hdr_iter->next = new_h;
    }

    if (new_h->asize == 0 && hdr_should_split(new_h, size)) {
        hdr_split(new_h, size);
    } else {
        new_h->asize = size;
    }

    return (void *)((char *)new_h + sizeof(Header));
}

/**
 * Free memory block.
 * @param ptr       pointer to previously allocated data
 * @pre ptr != NULL
 */
void mfree(void *ptr)
{
    assert(ptr != NULL);
    Header *freed = (Header *)((char *)ptr - sizeof(Header));
    freed->asize = 0;
    if (hdr_can_merge(freed, freed->next)) 
        hdr_merge(freed, freed->next);
    if (hdr_can_merge(hdr_get_prev(freed), freed)) 
        hdr_merge(hdr_get_prev(freed), freed);
}

/**
 * Reallocate previously allocated block.
 * @param ptr       pointer to previously allocated data
 * @param size      a new requested size. Size can be greater, equal, or less
 * then size of previously allocated block.
 * @return pointer to reallocated space or NULL if size equals to 0.
 * @post header_of(return pointer)->size == size
 */
void *mrealloc(void *ptr, size_t size)
{
    if (size == 0){
        mfree(ptr);
        return NULL;
    }
    Header *hdr = ((Header *)ptr)-1;
    if (size == hdr->asize){
        return ptr;
    }
    if (size <= hdr->size){
        hdr->asize = 0;
        if(hdr_should_split(hdr, size)){
            hdr_split(hdr, size);
        }
        hdr->asize = size;
        return ptr;
    }
    void *ptr_new = mmalloc(size);
    if (ptr_new){
        memcpy(ptr_new, ptr, hdr->asize);
        mfree(ptr);
    }
    return ptr_new;
}
