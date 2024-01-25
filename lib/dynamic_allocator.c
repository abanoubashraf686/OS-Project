/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"
#include "../inc/stdio.h"
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
bool realloc_is_running = 0 ;
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1);
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_details_of_block(void* add ){
	struct BlockMetaData* blk =  META(add);
	cprintf("\n (size: %d, isFree: %d, address:%x) \n", blk->size, blk->is_free,blk+1) ;
}
void print_blocks_list(struct MemBlock_LIST *list)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	int idx = 0 ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, list)
	{
		cprintf("idx:%d -> (size: %d, isFree: %d, address:%x)\n", idx, blk->size, blk->is_free,blk+1) ;
		idx++ ;
	}
	cprintf("=========================================\n");
}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
bool valid_address(struct BlockMetaData *address){
	struct BlockMetaData *curr;
	LIST_FOREACH(curr, &free_blocks)
		if(curr == address)
			return 1;
	return 0;
}
int update_sbreak(int increment,uint32 *seg_break,const uint32 block_start,const uint32 hlimit){
	bool sign = (increment<0) ;
	uint32 old_seg_break = *seg_break ;
	uint32 new_break= old_seg_break +increment ;
	if(sign==0)
		new_break =ROUNDUP(new_break,PAGE_SIZE);
	if(new_break> hlimit || new_break<block_start){
		return -1;
	}
	*seg_break=new_break;
	return (sign==0?old_seg_break:*seg_break);
}
void delete_block(struct BlockMetaData*next){
	LIST_REMOVE(&free_blocks,next);
	next->is_free=0,
	next->size=0;
	next->prev_next_info.le_next=NULL,
	next->prev_next_info.le_prev =NULL;
}
void free_next(struct BlockMetaData*curr){
	struct BlockMetaData *next = LIST_NEXT(curr);
	if(next != NULL && next == next_address(curr))
		curr->size += next->size,delete_block(next);
}
void *alloc_and_split(struct BlockMetaData *curr,uint32 size){
	uint32 rem = curr->size-size ;
	curr->is_free =0 ,curr->size = size ;
	if(rem>=sizeOfMetaData()){
		struct BlockMetaData *new_block= (struct BlockMetaData *)(((uint32)curr)+size);
		new_block->size = rem ,new_block->is_free =1 ;
		LIST_INSERT_AFTER(&free_blocks,curr,new_block);
	}
	else{
		curr->size+=rem;
	}
	//	new
	LIST_REMOVE(&free_blocks,curr);
	return ++curr;
}
void *call_sbreak(uint32 size){
	if(size>2147483647)
		return NULL;
	void *old_sbrk = sbrk(size) ;
	if(old_sbrk==(void*)-1)
		return NULL ;
	struct BlockMetaData* last = old_sbrk;
	last->size = (uint32)(sbrk(0)-old_sbrk);
	last->is_free=1;
	LIST_INSERT_TAIL(&free_blocks,last);
	return alloc_and_split(last,size);
}
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace==0)
		return ;
	//=========================================
	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator() ✅
	if(initSizeOfAllocatedSpace>=sizeOfMetaData()){
		is_initialized=1 ;
		memset((void*)daStart,0,initSizeOfAllocatedSpace);
		struct BlockMetaData *head_metadata = (struct BlockMetaData*)daStart;
		head_metadata->size = initSizeOfAllocatedSpace;
		head_metadata->is_free = 1;
		LIST_INSERT_HEAD(&free_blocks,head_metadata);
	}
}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF() ✅
	//	panic("alloc_block_FF is not implemented yet");
	if(size == 0 || size>UINT_MAX-sizeOfMetaData())
		return NULL;
	size += sizeOfMetaData();
	if( (is_initialized)==0){
		uint32 st = (uint32)sbrk(size);
		uint32 en = (uint32)sbrk(0);
		initialize_dynamic_allocator(st,en-st);
	}
	struct BlockMetaData *curr;
	LIST_FOREACH(curr, &free_blocks){
		 if(curr->size >= size){
			return alloc_and_split(curr, size);
		 }
	}
	return call_sbreak(size) ;
}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF() ✅
	if(size == 0 || size>UINT_MAX-sizeOfMetaData())
		return NULL;

	size+=sizeOfMetaData();
	if(is_initialized==0){
		uint32 st = (uint32)sbrk(size);
		uint32 en = (uint32)sbrk(0);
		initialize_dynamic_allocator(st,en-st);
	}
	struct BlockMetaData *curr, *will_alloc=NULL;

	LIST_FOREACH(curr, &free_blocks){
		 if(curr->size >= size && (will_alloc == NULL || curr->size < will_alloc->size))
			 will_alloc = curr;
	}
	if(will_alloc != NULL)
		return alloc_and_split(will_alloc, size);
	return call_sbreak(size);
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
struct BlockMetaData* next_address(struct BlockMetaData*curr){
	struct BlockMetaData*ra =  (struct BlockMetaData*)( ((uint32)curr) +(curr->size) );
	return ra ;
}
void INSERT_IN_LIST(struct BlockMetaData *curr){
	if(curr==NULL || curr->is_free==1)
		return ;
	curr->is_free =1 ;
	struct BlockMetaData *i;
	for(i =free_blocks.lh_first ;i!=NULL;i =LIST_NEXT(i)){
		if(i>curr){
			LIST_INSERT_BEFORE(&free_blocks,i,curr);
			return ;
		}
	}
	LIST_INSERT_TAIL(&free_blocks,curr);
}
void free_block(void *va)
{
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block() ✅
	if(va == NULL){
//		panic("Invalid address");
		return;
	}
	struct BlockMetaData *curr = META(va), *prev ;
	memset(va,0,curr->size-sizeOfMetaData());
	INSERT_IN_LIST(curr);
	free_next(curr);
	prev= LIST_PREV(curr);
	if(prev != NULL)
		free_next(prev);
}
//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF() ✅
	realloc_is_running=1 ;
	if (va == NULL){
		void* ra = alloc_block_FF(new_size);
		realloc_is_running=0;
		return ra ;
	}
	else if (new_size == 0){
		free_block(va);
		realloc_is_running=0;
		return NULL;
	}
	new_size+=sizeOfMetaData();
	struct BlockMetaData *curr = META(va) ;
	if(new_size==curr->size){
		realloc_is_running=0;
		return va ;
	}
	uint32 oldsz = curr->size ;
	INSERT_IN_LIST(curr);
	free_next(curr);
	void *ra;
	if((curr->size)<new_size)
	{
		struct BlockMetaData *prev= LIST_PREV(curr);
		bool completely_free;
		if(prev != NULL && next_address(prev)==curr){
			if((prev->size+curr->size)>=new_size)
				free_next(prev),completely_free=1;
			else
				completely_free=0;
		}
		else
			completely_free=1 ;
		ra = alloc_block_FF(new_size-sizeOfMetaData());
		if(ra==NULL){
			alloc_and_split(curr,oldsz);
			realloc_is_running=0 ;
			return ra ;
		}
		if(!completely_free){
			free_next(prev);
			completely_free=1 ;
		}
	}
	else
		ra =  alloc_and_split(curr, new_size);
	if(ra!=va)
		memcpy(ra,va,(oldsz>new_size?new_size:oldsz)-sizeOfMetaData());
	realloc_is_running=0 ;
	return ra ;
}
///============================================================================================
//										Converting
uint32 NumOfPage(uint32 va){
	return (va-block_start)/PAGE_SIZE;
}
uint32 addOfpage(uint32 num){
	return block_start+num*PAGE_SIZE;
}
uint32 get_size(uint32 curr){
	return block_pages[curr].number_of_pages&(~(1<<31));
}
uint32 next_page(uint32 idx){
	return idx + get_size(idx);
}
uint32 convertSizeToNumberOfPages(uint32 size){
	return (size>>12)+((size&(PAGE_SIZE-1))!=0);
}
uint32 get_prev(uint32 curr){
	if(curr<=NumOfPage(page_start))
		return -1 ;
	return curr-get_size(curr-1) ;
}
void set_prev_of_the_next(uint32 curr){
	uint32 last = next_page(curr)-1 ;
	block_pages[last].number_of_pages = block_pages[curr].number_of_pages;
}
bool is_free(uint32 curr){
	return (block_pages[curr].number_of_pages&(1<<31))!=0 ;
}
void set_free(uint32 curr){
	block_pages[curr].number_of_pages|=(1<<31);
}
void un_free(uint32 curr){
	block_pages[curr].number_of_pages = get_size(curr) ;
}
//============================================================================================
//									Special double Linked list
void delete_block_page(uint32 curr){
	block_pages[curr].number_of_pages=(1<<31);
}
void merge_Next(uint32 curr){
	uint32 next = next_page(curr) ;
	if(next<num_of_pages && is_free(next)){
		block_pages[curr].number_of_pages+=get_size(next);
		set_prev_of_the_next(curr);
		delete_block_page(next);
	}
}
void free_block_page(uint32 curr){
	struct block_page *block = &block_pages[curr];
	set_free(curr);
	merge_Next(curr);
	int prev = get_prev(curr) ;
	if(prev!=-1 && is_free(prev)){
		merge_Next(prev);
	}
}
void AllocAndSplit_blockPage(uint32 curr,uint32 needed_pages){
	uint32 new_block =curr+needed_pages;
	block_pages[curr].number_of_pages = get_size(curr);
	if(new_block!=next_page(curr)){
		block_pages[new_block].number_of_pages = get_size(curr)-needed_pages;
		set_free(new_block);
		set_prev_of_the_next(new_block);
	}
	block_pages[curr].number_of_pages = needed_pages ;
	set_prev_of_the_next(curr);
}
int search_needed_pagesFF(uint32 needed_pages){
	for(uint32 curr = NumOfPage(page_start);curr<num_of_pages;curr=next_page(curr))
			if(is_free(curr) && get_size(curr)>=needed_pages)
				return curr ;
	return -1 ;
}
int search_needed_pagesBF(uint32 needed_pages){
	int will_allocate = -1 ;
	for(uint32 curr = NumOfPage(page_start);curr<num_of_pages;curr=next_page(curr))
		if(is_free(curr) && get_size(curr)>=needed_pages && (will_allocate==-1 || get_size(curr)<get_size(will_allocate)))
			will_allocate=curr ;
	return will_allocate;
}
void initialize_block_list(){
	if(block_pages[NumOfPage(page_start)].number_of_pages==0){
	block_pages[NumOfPage(page_start)].number_of_pages =num_of_pages-NumOfPage(page_start);
	set_prev_of_the_next(NumOfPage(page_start));
	for(uint32 i = 0 ;i<num_of_pages ;i++)
		set_free(i);
	}
}
void initialize_page_allocator(struct block_page *curr_block_page,uint32 st,uint32 brk,uint32 en,uint32 N){
	block_pages = curr_block_page;
	block_start = st , seg_break =brk ,hlimit = en ,num_of_pages = N ;
	page_start = en + PAGE_SIZE ;
}
