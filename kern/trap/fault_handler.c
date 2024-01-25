/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}
void allocate_frame_and_validate(struct Env * curenv, uint32 fault_va,struct FrameInfo **ptr_frame){
	*ptr_frame = Map_virtual(curenv->env_page_directory,fault_va,PERM_USER|PERM_WRITEABLE);
		 if (((pf_read_env_page(curenv, (void*)fault_va)) == E_PAGE_NOT_EXIST_IN_PF) &&
						(fault_va>=USER_HEAP_MAX|| fault_va<USER_HEAP_START) &&
						(fault_va>=USTACKTOP || fault_va<USTACKBOTTOM) ){
			 unmap_frame(curenv->env_page_directory,fault_va);
							sched_kill_env(curenv->env_id);
		 }
}
void transfer_last_active_to_seconed_list(struct Env * curenv){
	struct WorkingSetElement *last_active =LIST_LAST(&curenv->ActiveList) ;
	LIST_REMOVE(&curenv->ActiveList,last_active);
	pt_set_page_permissions(curenv->env_page_directory,last_active->virtual_address,0,PERM_PRESENT) ;
	LIST_INSERT_HEAD(&curenv->SecondList,last_active);
}
void transfer_first_second_to_active_list(struct Env * curenv){
	struct WorkingSetElement *first_second =LIST_FIRST(&curenv->SecondList) ;
	LIST_REMOVE(&curenv->SecondList,first_second);
	pt_set_page_permissions(curenv->env_page_directory,first_second->virtual_address,PERM_PRESENT,0) ;
	LIST_INSERT_TAIL(&curenv->ActiveList,first_second);
}
void remove_victim(struct Env * curenv,uint32 va_victim){
	uint32 *ptr_page_table ;
	struct FrameInfo *victim_frame = get_frame_info(curenv->env_page_directory,va_victim,&ptr_page_table) ;
	if(pt_get_page_permissions(curenv->env_page_directory,va_victim)&PERM_MODIFIED){
		pf_update_env_page(curenv,va_victim,victim_frame);
	}
	unmap_frame(curenv->env_page_directory,va_victim);
}
//Handle the page fault
void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif
	//cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
	//refer to the project presentation and documentation for details
	fault_va = ROUNDDOWN(fault_va,PAGE_SIZE);
	struct FrameInfo *ptr_frame=NULL;
	if(isPageReplacmentAlgorithmFIFO())
	{
		allocate_frame_and_validate(curenv,fault_va,&ptr_frame);
		if(curenv->page_WS_list.size==0 || (curenv->page_WS_list.size!=(curenv->page_WS_max_size)))
		{
			//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement ✅
			struct WorkingSetElement *ws = env_page_ws_list_create_element(curenv, fault_va);

			LIST_INSERT_TAIL(&(curenv->page_WS_list),ptr_frame->element);
			if (LIST_SIZE(&(curenv->page_WS_list)) == curenv->page_WS_max_size)
				curenv->page_last_WS_element = LIST_FIRST(&(curenv->page_WS_list));
		}
		else{
			//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement ✅
			victimWSElement = LIST_FIRST(&(curenv->page_WS_list)) ;
			uint32 va_victim = victimWSElement->virtual_address;
			remove_victim(curenv,va_victim);

			victimWSElement->virtual_address=fault_va ;
			ptr_frame->environment = curenv, ptr_frame->element = victimWSElement;

			LIST_REMOVE(&curenv->page_WS_list,victimWSElement);
			LIST_INSERT_TAIL(&curenv->page_WS_list,victimWSElement);
			if(curenv->page_WS_list.size==curenv->page_WS_max_size)
				curenv->page_last_WS_element=LIST_FIRST(&curenv->page_WS_list);
		}
}
	else if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX)){
		uint32 sz_active=LIST_SIZE(&curenv->ActiveList),sz_second =LIST_SIZE(&curenv->SecondList) ;
		if(sz_second!=0){
			uint32 *ptr_page_table ;
			ptr_frame = get_frame_info(curenv->env_page_directory,fault_va,&ptr_page_table);
			// 1- In second list
			if(ptr_frame!=NULL && ptr_frame->element!=NULL && ptr_frame->element->virtual_address==fault_va){
					transfer_last_active_to_seconed_list(curenv);
					LIST_REMOVE(&curenv->SecondList,ptr_frame->element);
					pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0) ;
					LIST_INSERT_HEAD(&curenv->ActiveList,ptr_frame->element);
					return ;
			}
		}
		allocate_frame_and_validate(curenv,fault_va,&ptr_frame);
		sz_active=LIST_SIZE(&curenv->ActiveList),sz_second =LIST_SIZE(&curenv->SecondList);
		if(sz_active+sz_second==0 || (sz_active+sz_second!=curenv->page_WS_max_size))
		{

			//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU placement ✅
			struct WorkingSetElement *ws = env_page_ws_list_create_element(curenv, fault_va);

			if(sz_active==curenv->ActiveListSize){
				transfer_last_active_to_seconed_list(curenv);
			}

			LIST_INSERT_HEAD(&curenv->ActiveList,ptr_frame->element);
		}
		else
		{
			//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement- O(1) implementation of LRU replacement ✅
			if(sz_second!=0){
				victimWSElement = LIST_LAST(&curenv->SecondList) ;
				LIST_REMOVE(&curenv->SecondList,victimWSElement);
				transfer_last_active_to_seconed_list(curenv);
			}
			else{
				victimWSElement = LIST_LAST(&curenv->ActiveList);
				LIST_REMOVE(&curenv->ActiveList,victimWSElement);
			}
			remove_victim(curenv,victimWSElement->virtual_address);

			victimWSElement->virtual_address=fault_va ;
			ptr_frame->environment = curenv, ptr_frame->element = victimWSElement;
			LIST_INSERT_HEAD(&curenv->ActiveList,victimWSElement);
		}
	}
}

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}



