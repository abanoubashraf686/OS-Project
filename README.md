# FOS
Fcis OS is a like [MIT_JOS](https://pdos.csail.mit.edu/6.828/2014/overview.html) <br/>
the change mainly made in the Memory Managment Part and cpu schedular<br/>
TODO List:
		Description	Resource	Path	Location	Type
		TODO [PROJECT'23.MS1 - #0 GIVENS] DYNAMIC ALLOCATOR helper functions ✅	dynamic_allocator.h	/FOS_PROJECT_2023_TEMPLATE/inc	line 61	C/C++ Task
		TODO [PROJECT'23.MS1 - #2] [1] PLAY WITH CODE! - process_command ✅	command_prompt.c	/FOS_PROJECT_2023_TEMPLATE/kern/cmd	line 379	C/C++ Task
		TODO [PROJECT'23.MS1 - #3] [2] SYSTEM CALLS - Add suitable code here ✅	syscall.h	/FOS_PROJECT_2023_TEMPLATE/inc	line 55	C/C++ Task
		TODO [PROJECT'23.MS1 - #3] [2] SYSTEM CALLS - Implement these system calls ✅	syscall.c	/FOS_PROJECT_2023_TEMPLATE/lib	line 336	C/C++ Task
		TODO [PROJECT'23.MS1 - #4] [2] SYSTEM CALLS - Add suitable code here ✅	syscall.c	/FOS_PROJECT_2023_TEMPLATE/kern/trap	line 504	C/C++ Task
		TODO [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator() ✅	dynamic_allocator.c	/FOS_PROJECT_2023_TEMPLATE/lib	line 156	C/C++ Task
		TODO [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF() ✅	dynamic_allocator.c	/FOS_PROJECT_2023_TEMPLATE/lib	line 172	C/C++ Task
		TODO [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block() ✅	dynamic_allocator.c	/FOS_PROJECT_2023_TEMPLATE/lib	line 256	C/C++ Task
		TODO [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF() ✅	dynamic_allocator.c	/FOS_PROJECT_2023_TEMPLATE/lib	line 274	C/C++ Task
		TODO [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF() ✅	dynamic_allocator.c	/FOS_PROJECT_2023_TEMPLATE/lib	line 195	C/C++ Task
		TODO [PROJECT'23.MS2 - #0 GIVENS] [PROGRAM LOAD] create_page_table() ✅	memory_manager.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 333	C/C++ Task
		TODO [PROJECT'23.MS2 - #0 GIVENS] [PROGRAM LOAD] create_user_directory() ✅	user_environment.c	/FOS_PROJECT_2023_TEMPLATE/kern/proc	line 792	C/C++ Task
		TODO [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialization: add suitable code here ✅	kheap.h	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 30	C/C++ Task
		TODO [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator() ✅	kheap.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 8	C/C++ Task
		TODO [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk() ✅	kheap.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 33	C/C++ Task
		TODO [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc() ✅	kheap.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 84	C/C++ Task
		TODO [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree() ✅	kheap.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 111	C/C++ Task
		TODO [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address() ✅	kheap.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 127	C/C++ Task
		TODO [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address() ✅	kheap.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 138	C/C++ Task
		TODO [PROJECT'23.MS2 - #07] [2] USER HEAP - initialize of Env: add suitable code here ✅	environment_definitions.h	/FOS_PROJECT_2023_TEMPLATE/inc	line 110	C/C++ Task
		TODO [PROJECT'23.MS2 - #07] [2] USER HEAP - initialize_uheap_dynamic_allocator() ✅	user_environment.c	/FOS_PROJECT_2023_TEMPLATE/kern/proc	line 813	C/C++ Task
		TODO [PROJECT'23.MS2 - #08] [2] USER HEAP - Block Allocator - sys_sbrk() [Kernel Side] ✅	syscall.c	/FOS_PROJECT_2023_TEMPLATE/kern/trap	line 479	C/C++ Task
		TODO [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side] ✅	uheap.c	/FOS_PROJECT_2023_TEMPLATE/lib	line 42	C/C++ Task
		TODO [PROJECT'23.MS2 - #10] [2] USER HEAP - allocate_user_mem() [Kernel Side] ✅	chunk_operations.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 121	C/C++ Task
		TODO [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side] ✅	uheap.c	/FOS_PROJECT_2023_TEMPLATE/lib	line 65	C/C++ Task
		TODO [PROJECT'23.MS2 - #12] [2] USER HEAP - free_user_mem() [Kernel Side] ✅	chunk_operations.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 130	C/C++ Task
		TODO [PROJECT'23.MS2 - #13] [3] PAGE FAULT HANDLER - Check for invalid pointers ✅	trap.c	/FOS_PROJECT_2023_TEMPLATE/kern/trap	line 382	C/C++ Task
		TODO [PROJECT'23.MS2 - #14] [3] PAGE FAULT HANDLER - Create a new working set element ✅	working_set_manager.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 18	C/C++ Task
		TODO [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement ✅	fault_handler.c	/FOS_PROJECT_2023_TEMPLATE/kern/trap	line 121	C/C++ Task
		TODO [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc() ✅	kheap.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 181	C/C++ Task
		TODO [PROJECT'23.MS2 - BONUS#2] [2] USER HEAP - free_user_mem() IN O(1) ✅	chunk_operations.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 131	C/C++ Task
		TODO [PROJECT'23.MS3 - #0 GIVENS] [1] FAULT HANDLER REPLACEMENT - Data structures of LRU Approx replacement policy ✅	environment_definitions.h	/FOS_PROJECT_2023_TEMPLATE/inc	line 135	C/C++ Task
		TODO [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement ✅	fault_handler.c	/FOS_PROJECT_2023_TEMPLATE/kern/trap	line 129	C/C++ Task
		TODO [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU placement ✅	fault_handler.c	/FOS_PROJECT_2023_TEMPLATE/kern/trap	line 162	C/C++ Task
		TODO [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement- O(1) implementation of LRU replacement ✅	fault_handler.c	/FOS_PROJECT_2023_TEMPLATE/kern/trap	line 173	C/C++ Task
		TODO [PROJECT'23.MS3 - #3] [2] BSD SCHEDULER - env_get_nice ✅	sched_helpers.c	/FOS_PROJECT_2023_TEMPLATE/kern/cpu	line 660	C/C++ Task
		TODO [PROJECT'23.MS3 - #3] [2] BSD SCHEDULER - env_get_recent_cpu ✅	sched_helpers.c	/FOS_PROJECT_2023_TEMPLATE/kern/cpu	line 672	C/C++ Task
		TODO [PROJECT'23.MS3 - #3] [2] BSD SCHEDULER - env_set_nice ✅	sched_helpers.c	/FOS_PROJECT_2023_TEMPLATE/kern/cpu	line 665	C/C++ Task
		TODO [PROJECT'23.MS3 - #3] [2] BSD SCHEDULER - get_load_average ✅	sched_helpers.c	/FOS_PROJECT_2023_TEMPLATE/kern/cpu	line 677	C/C++ Task
		TODO [PROJECT'23.MS3 - #4] [2] BSD SCHEDULER - sched_init_BSD ✅	sched.c	/FOS_PROJECT_2023_TEMPLATE/kern/cpu	line 164	C/C++ Task
		TODO [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - fos_scheduler_BSD ✅	sched.c	/FOS_PROJECT_2023_TEMPLATE/kern/cpu	line 205	C/C++ Task
		TODO [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - Your code is here ✅	sched.c	/FOS_PROJECT_2023_TEMPLATE/kern/cpu	line 248	C/C++ Task
		TODO [PROJECT'23.MS3 - BONUS] EXIT ENV: env_free ✅	user_environment.c	/FOS_PROJECT_2023_TEMPLATE/kern/proc	line 458	C/C++ Task
		TODO [PROJECT'23.MS3 - BONUS] Free RAM when it's FULL ✅	memory_manager.c	/FOS_PROJECT_2023_TEMPLATE/kern/mem	line 193	C/C++ Task
