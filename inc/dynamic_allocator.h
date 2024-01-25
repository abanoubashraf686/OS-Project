#ifndef FOS_INC_DYNBLK_MANAGE_H
#define FOS_INC_DYNBLK_MANAGE_H
#include <inc/queue.h>
#include <inc/types.h>
#include <inc/environment_definitions.h>
#define META(va) ((struct BlockMetaData*)va)-1

struct MemBlock_LIST free_blocks;
bool is_initialized ;
extern bool realloc_is_running ;
struct BlockMetaData* next_address(struct BlockMetaData*curr);
void INSERT_IN_LIST(struct BlockMetaData *curr);
uint32 num_of_pages,page_start,seg_break,hlimit,block_start;
struct block_page *block_pages;
uint32 NumOfPage(uint32 va);
uint32 addOfpage(uint32 num);
uint32 next_page(uint32 idx);
uint32 get_prev(uint32 curr);
void set_prev_of_the_next(uint32 curr);
bool is_free(uint32 curr);
void set_free(uint32 curr);
void un_free(uint32 curr);
uint32 get_size(uint32 curr);
uint32 convertSizeToNumberOfPages(uint32 size);
void delete_block_page(uint32 curr);
void merge_Next(uint32 curr);
void free_block_page(uint32 curr);
void AllocAndSplit_blockPage(uint32 curr,uint32 needed_pages);
int search_needed_pagesFF(uint32 needed_pages);
int search_needed_pagesBF(uint32 needed_pages);
void* Gen_sbrk(int increment);
void initialize_page_allocator(struct block_page *curr_block_page,uint32 st,uint32 brk,uint32 en,uint32 N);
void initialize_block_list();
int update_sbreak(int increment,uint32 *seg_break,const uint32 block_start,const uint32 hlimit);
void adjust_blocks(uint32 seg_break);
/*Data*/
/*Max Size for the Dynamic Allocator*/
#define DYN_ALLOC_MAX_SIZE (32<<20) //32MB
#define DYN_ALLOC_MAX_BLOCK_SIZE (1<<11) //2KB
//  address min_address_of_size[]
//  list
/*Allocation Type*/
enum
{
	DA_FF = 1,
	DA_NF,
	DA_BF,
	DA_WF
};
#define sizeOfMetaData() (sizeof(struct BlockMetaData))


/*Functions*/

/*2024*/
//should be implemented inside kern/mem/kheap.c
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit);
//should be implemented inside kern/proc/user_environment.c
void initialize_uheap_dynamic_allocator(struct Env* env, uint32 daStart, uint32 daLimit);

//TODO: [PROJECT'23.MS1 - #0 GIVENS] DYNAMIC ALLOCATOR helper functions ✅
uint32 get_block_size(void* va);
int8 is_free_block(void* va);
void print_blocks_list(struct MemBlock_LIST *list);
//===================================================================

//Required Functions
//In KernelHeap: should be implemented inside kern/mem/kheap.c
//In UserHeap: should be implemented inside lib/uheap.c
void* sbrk(int increment);
void print_details_of_block(void* add );
bool valid_address(struct BlockMetaData *address);
void delete_block(struct BlockMetaData*next);
void free_next(struct BlockMetaData*curr);
void *alloc_and_split(struct BlockMetaData *curr,uint32 size);
void *call_sbreak(uint32 size);
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace);
void *alloc_block(uint32 size, int ALLOC_STRATEGY);
void *alloc_block_FF(uint32 size);
void *alloc_block_BF(uint32 size);
void *alloc_block_WF(uint32 size);
void *alloc_block_NF(uint32 size);
void free_block(void* va);
void *realloc_block_FF(void* va, uint32 new_size);

#endif
