#include "kheap.h"
#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
struct block_page kernal_block_pages[NUM_OF_KHEAP_PAGES];
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator() ✅
	//(if no memory OR initial size exceed the given limit): E_NO_MEM
	if(daStart+initSizeToAllocate>daLimit || (uint32)KERNEL_HEAP_START>daStart || daLimit>(uint32)KERNEL_HEAP_MAX)
			return E_NO_MEM;
	kernal_block_start = daStart , kernal_s_break=daStart+initSizeToAllocate,kernal_hard_limit=daLimit ;
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	initialize_page_allocator(kernal_block_pages,kernal_block_start,kernal_s_break,kernal_hard_limit,NUM_OF_KHEAP_PAGES);
	initialize_block_list();
	//All pages in the given range should be allocated
	for(uint32 va = daStart;va<kernal_s_break;va+=PAGE_SIZE)
	{
		struct FrameInfo* mapped_fram = Map_virtual(ptr_page_directory,va,PERM_WRITEABLE|PERM_AVAILABLE);
		if(mapped_fram==NULL)
			return E_NO_MEM;
	}
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	initialize_dynamic_allocator(daStart,initSizeToAllocate);
	//Return:
	//	On success: 0
	return 0;
}

void* sbrk(int increment)
{
//	initialize_page_allocator(kernal_block_pages,kernal_block_start,kernal_s_break,kernal_hard_limit,NUM_OF_KHEAP_PAGES);
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk() ✅
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */
	void *ra  ;
	if(increment==0)
		return (void*)kernal_s_break;
	else {
		uint32 old_sbrk = kernal_s_break ;
		void* res = (void*)update_sbreak(increment,&kernal_s_break,kernal_block_start,kernal_hard_limit);
		if(res==(void*)-1){
			if(realloc_is_running==1)
				return res ;
			else
				panic("sbrk failed");
		}
		if(increment>0)
			map_range(ROUNDUP(old_sbrk,PAGE_SIZE),kernal_s_break);
		else
			unmap_range(ROUNDUP(kernal_s_break,PAGE_SIZE),ROUNDUP(old_sbrk,PAGE_SIZE));
		return res ;
	}
}
void* Alloc_numberOfPages(uint32 needed_pages){
	int curr ;
	if(isKHeapPlacementStrategyFIRSTFIT())
		curr= search_needed_pagesFF(needed_pages);
	else if(isKHeapPlacementStrategyBESTFIT())
		curr=search_needed_pagesBF(needed_pages);
	if(curr==-1)
		return NULL ;
	AllocAndSplit_blockPage(curr,needed_pages);
//	cprintf("\nAllocated %d pages start with page number %d\n",needed_pages,curr);
	map_range(addOfpage(curr),addOfpage(curr+needed_pages));
	return (void*)addOfpage(curr);
}
bool test_realloc = 0 ;
void* kmalloc(unsigned int size)
{
//	initialize_page_allocator(kernal_block_pages,kernal_block_start,kernal_s_break,kernal_hard_limit,NUM_OF_KHEAP_PAGES);
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc() ✅
	if(size==0 || size>=(KERNEL_HEAP_MAX-page_start))
		return NULL;
	if(size<=DYN_ALLOC_MAX_BLOCK_SIZE){
		if(isKHeapPlacementStrategyFIRSTFIT()==1){
			return alloc_block_FF(size);
		}
		else if (isKHeapPlacementStrategyBESTFIT()==1){
			return alloc_block_BF(size);
		}
	}
	else if(isKHeapPlacementStrategyFIRSTFIT()==1 || isKHeapPlacementStrategyBESTFIT()){
//		if(test_realloc==0){
//			test_realloc=1 ;
//			cprintf("\n Size: %d \n",size);
//			return krealloc(kmalloc(num_of_pages*PAGE_SIZE),size);
//		}
		return Alloc_numberOfPages(convertSizeToNumberOfPages(size));
	}
	else
		panic("not implemented!");
	return NULL;
}

void kfree(void* virtual_address)
{
//	initialize_page_allocator(kernal_block_pages,kernal_block_start,kernal_s_break,kernal_hard_limit,NUM_OF_KHEAP_PAGES);
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree() ✅
	uint32 va = (uint32)virtual_address;
	if(va>=kernal_block_start && va<kernal_s_break){
		free_block(virtual_address);
	}
	else if(va>=page_start && va<KERNEL_HEAP_MAX && ((va-kernal_block_start)&(PAGE_SIZE-1))==0){
		uint32 curr = NumOfPage(va);
		unmap_range(addOfpage(curr),addOfpage(next_page(curr))),
		free_block_page(curr);
	}
	else
		panic("Invalid address");
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address() ✅
	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
	uint32 off = physical_address&4095 ;
	struct FrameInfo*ptr_frame =  to_frame_info(physical_address&4294963200);
	if(ptr_frame!=NULL && ptr_frame->va>=kernal_block_start && ptr_frame->va<KERNEL_HEAP_MAX)
		return ptr_frame->va+off ;
	return 0 ;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address() ✅
	uint32 *ptr_page_table = NULL;
	uint32 off = virtual_address&4095;
	virtual_address &=4294963200;
	struct FrameInfo* frame= get_frame_info(ptr_page_directory,virtual_address,&ptr_page_table);
	if(frame!=NULL)
		return to_physical_address(frame)+off;
	return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.
//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().
void *krealloc(void *virtual_address, uint32 new_size)
{
//	initialize_page_allocator(kernal_block_pages,kernal_block_start,kernal_s_break,kernal_hard_limit,NUM_OF_KHEAP_PAGES);
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc() ✅
	if (virtual_address == NULL){
		realloc_is_running=1 ;
		void*ra =  kmalloc(new_size);
		realloc_is_running=0 ;
		return ra ;
	}
	if (new_size == 0){
		kfree(virtual_address);
		return NULL;
	}
	uint32 va = (uint32)virtual_address;
	uint32 mn_sz  ;
	if(va>=kernal_block_start && va<kernal_s_break){
		if (new_size <= DYN_ALLOC_MAX_BLOCK_SIZE)
			return realloc_block_FF(virtual_address,new_size);

		mn_sz = get_block_size(virtual_address)-sizeOfMetaData();
		// from block to pages
		void*ra =  kmalloc(new_size);
		if(ra!=NULL){
//				cprintf("\n Try to copy data! \n");
			memcpy(ra,virtual_address,mn_sz);
			kfree(virtual_address);
		}
		return ra ;
	}
	else if (!(va>=page_start && va<KERNEL_HEAP_MAX && ((va-kernal_block_start)&(PAGE_SIZE-1))==0))
		panic("Invalid address");
	else if (new_size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		// from pages to block
		mn_sz = new_size ;
		realloc_is_running=1 ;
		void*ra =  kmalloc(new_size);
		realloc_is_running=0 ;
		if(ra!=NULL){
//			cprintf("\n Try to copy data! \n");
			memcpy(ra,virtual_address,mn_sz);
			kfree(virtual_address);
		}
		return ra ;
	}
	int curr = NumOfPage(va), Old_next = next_page(curr);
	uint32 needed_pages = convertSizeToNumberOfPages(new_size), old_num_page = get_size(curr) ;
//	cprintf("\n current page: %d, needed pages: %d,old:%d \n",curr-NumOfPage(page_start),needed_pages,old_num_page);
//	for(uint32 curr = NumOfPage(page_start);curr<num_of_pages;curr=next_page(curr))
//		cprintf("\n Page:%d			size: %d pages			is_free: %d \n",curr-NumOfPage(page_start),get_size(curr),is_free(curr));
	if(needed_pages==old_num_page){
		return virtual_address ;
	}
	mn_sz = old_num_page  ;
	if(mn_sz>needed_pages)
		mn_sz = needed_pages;
	//partially free
	set_free(curr);
	merge_Next(curr);
	if(needed_pages<=get_size(curr))
	{
//		cprintf("\n extended !\n");
		AllocAndSplit_blockPage(curr,needed_pages);
		if(needed_pages<old_num_page){
			unmap_range(addOfpage(next_page(curr)),addOfpage(Old_next));
		}
		else
			map_range(addOfpage(Old_next), addOfpage(next_page(curr)));
//		for(uint32 curr = NumOfPage(page_start);curr<num_of_pages;curr=next_page(curr))
//			cprintf("\n Page:%d			size: %d pages			is_free: %d \n",curr-NumOfPage(page_start),get_size(curr),is_free(curr));
		return virtual_address;
	}
	// granted that the new size is greater than the old
	//1- completely free if we always can allocate virtually!
	int prev = get_prev(curr) ;
	bool completely_free;
	if(prev!=-1 && is_free(prev)==1){
		// check if we can or not
		if(get_size(prev)+get_size(curr)>=needed_pages){
			merge_Next(prev);
			completely_free=1 ;
		}
		else
			completely_free=0 ;
	}
	else
		completely_free =1 ;
	//2- search for new space
	int new_place = search_needed_pagesFF(needed_pages);
	// if it doesn't exist return NULL and keep it valid
	if(new_place==-1){
		//allocate it again
		AllocAndSplit_blockPage(curr,old_num_page);
		return NULL ;
	}

	// completely free & allocate virtually !
	if(!completely_free){
		merge_Next(prev);
		completely_free =1 ;
	}
	AllocAndSplit_blockPage(new_place,needed_pages);
	// get the start and the end
	uint32 new_va  = addOfpage(new_place) ,en = addOfpage(next_page(new_place));
	//mn_sz = min number of pages
	for(int i=0 ;i<mn_sz;i++){
		// copy frame frame to avoid taking more frames!
		Map_virtual(ptr_page_directory,new_va,PERM_AVAILABLE|PERM_WRITEABLE);
		memcpy((void*)new_va,(void*)va,PAGE_SIZE);
		unmap_frame(ptr_page_directory,va);
		new_va+=PAGE_SIZE,va+=PAGE_SIZE;
	}
	// map remaining frames !
	while(new_va!=en)
		Map_virtual(ptr_page_directory,new_va,PERM_AVAILABLE|PERM_WRITEABLE),new_va+=PAGE_SIZE;
//	for(uint32 curr = NumOfPage(page_start);curr<num_of_pages;curr=next_page(curr))
//		cprintf("\n Page:%d			size: %d pages			is_free: %d \n",curr-NumOfPage(page_start),get_size(curr),is_free(curr));

	// return address
	return (void*)addOfpage(new_place) ;
}
