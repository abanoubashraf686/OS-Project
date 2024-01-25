#include "sched.h"

#include <inc/assert.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/trap.h>
#include <kern/mem/kheap.h>
#include <kern/mem/memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/cmd/command_prompt.h>
//void on_clock_update_WS_time_stamps();
extern void cleanup_buffers(struct Env* head);
//================

//=================================================================================//
//============================== QUEUE FUNCTIONS ==================================//
//=================================================================================//

//================================
// [1] Initialize the given queue:
//================================
void init_queue(struct Env_Queue* queue)
{
	if(queue != NULL)
	{
		LIST_INIT(queue);
	}
}

//================================
// [2] Get queue size:
//================================
int queue_size(struct Env_Queue* queue)
{
	if(queue != NULL)
	{
		return LIST_SIZE(queue);
	}
	else
	{
		return 0;
	}
}

//====================================
// [3] Enqueue env in the given queue:
//====================================
void enqueue(struct Env_Queue* queue, struct Env* env)
{
	assert(queue != NULL)	;
	if(env != NULL)
	{
		LIST_INSERT_HEAD(queue, env);
	}
}

//======================================
// [4] Dequeue env from the given queue:
//======================================
struct Env* dequeue(struct Env_Queue* queue)
{
	if (queue == NULL) return NULL;
	struct Env* envItem = LIST_LAST(queue);
	if (envItem != NULL)
	{
		LIST_REMOVE(queue, envItem);
	}
	return envItem;
}

//====================================
// [5] Remove env from the given queue:
//====================================
void remove_from_queue(struct Env_Queue* queue, struct Env* e)
{
	assert(queue != NULL)	;

	if (e != NULL)
	{
		LIST_REMOVE(queue, e);
	}
}

//========================================
// [6] Search by envID in the given queue:
//========================================
struct Env* find_env_in_queue(struct Env_Queue* queue, uint32 envID)
{
	if (queue == NULL) return NULL;

	struct Env * ptr_env=NULL;
	LIST_FOREACH(ptr_env, queue)
	{
		if(ptr_env->env_id == envID)
		{
			return ptr_env;
		}
	}
	return NULL;
}

//=====================================================================================//
//============================== SCHED Q'S FUNCTIONS ==================================//
//=====================================================================================//

//========================================
// [1] Delete all ready queues:
//========================================
void sched_delete_ready_queues()
{
#if USE_KHEAP
	if (env_ready_queues != NULL)
		kfree(env_ready_queues);
	if (quantums != NULL)
		kfree(quantums);
#endif
}

//=================================================
// [2] Insert the given Env in the 1st Ready Queue:
//=================================================
void sched_insert_ready0(struct Env* env)
{
	if(env != NULL)
	{
		env->env_status = ENV_READY ;
		enqueue(&(env_ready_queues[0]), env);
	}
}

//=================================================
// [3] Remove the given Env from the Ready Queue(s):
//=================================================
void sched_remove_ready(struct Env* env)
{
	if(env != NULL)
	{
		for (int i = 0 ; i < num_of_ready_queues ; i++)
		{
			struct Env * ptr_env = find_env_in_queue(&(env_ready_queues[i]), env->env_id);
			if (ptr_env != NULL)
			{
				LIST_REMOVE(&(env_ready_queues[i]), env);
				env->env_status = ENV_UNKNOWN;
				return;
			}
		}
	}
}

//=================================================
// [4] Insert the given Env in NEW Queue:
//=================================================
void sched_insert_new(struct Env* env)
{
	if(env != NULL)
	{
		env->env_status = ENV_NEW ;
		enqueue(&env_new_queue, env);
	}
}

//=================================================
// [5] Remove the given Env from NEW Queue:
//=================================================
void sched_remove_new(struct Env* env)
{
	if(env != NULL)
	{
		LIST_REMOVE(&env_new_queue, env) ;
		env->env_status = ENV_UNKNOWN;
	}
}

//=================================================
// [6] Insert the given Env in EXIT Queue:
//=================================================
void sched_insert_exit(struct Env* env)
{
	if(env != NULL)
	{
		if(isBufferingEnabled()) {cleanup_buffers(env);}
		env->env_status = ENV_EXIT ;
		enqueue(&env_exit_queue, env);
	}
}
//=================================================
// [7] Remove the given Env from EXIT Queue:
//=================================================
void sched_remove_exit(struct Env* env)
{
	if(env != NULL)
	{
		LIST_REMOVE(&env_exit_queue, env) ;
		env->env_status = ENV_UNKNOWN;
	}
}

//=================================================
// [8] Sched the given Env in NEW Queue:
//=================================================
void sched_new_env(struct Env* e)
{
	//add the given env to the scheduler NEW queue
	if (e!=NULL)
	{
		sched_insert_new(e);
	}
}


//=================================================
// [9] Run the given EnvID:
//=================================================
void sched_run_env(uint32 envId)
{
	struct Env* ptr_env=NULL;
	LIST_FOREACH(ptr_env, &env_new_queue)
	{
		if(ptr_env->env_id == envId)
		{
			sched_remove_new(ptr_env);
			sched_insert_ready0(ptr_env);

			/*2015*///if scheduler not run yet, then invoke it!
			if (scheduler_status == SCH_STOPPED)
			{
				fos_scheduler();
			}
			break;
		}
	}
	//	cprintf("ready queue:\n");
	//	LIST_FOREACH(ptr_env, &env_ready_queue)
	//	{
	//		cprintf("%s - %d\n", ptr_env->prog_name, ptr_env->env_id);
	//	}

}

//=================================================
// [10] Exit the given EnvID:
//=================================================
void sched_exit_env(uint32 envId)
{
	struct Env* ptr_env=NULL;
	int found = 0;
	if (!found)
	{
		LIST_FOREACH(ptr_env, &env_new_queue)
		{
			if(ptr_env->env_id == envId)
			{
				sched_remove_new(ptr_env);
				found = 1;
				//			return;
			}
		}
	}
	if (!found)
	{
		for (int i = 0 ; i < num_of_ready_queues ; i++)
		{
			if (!LIST_EMPTY(&(env_ready_queues[i])))
			{
				ptr_env=NULL;
				LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
				{
					if(ptr_env->env_id == envId)
					{
						LIST_REMOVE(&(env_ready_queues[i]), ptr_env);
						found = 1;
						break;
					}
				}
			}
			if (found)
				break;
		}
	}
	if (!found)
	{
		if (curenv->env_id == envId)
		{
			ptr_env = curenv;
			found = 1;
		}
	}

	if (found)
	{
		sched_insert_exit(ptr_env);

		//If it's the curenv, then reinvoke the scheduler as there's no meaning to return back to an exited env
		if (curenv->env_id == envId)
		{
			curenv = NULL;
			fos_scheduler();
		}
	}
}


/*2015*/
//=================================================
// [11] KILL the given EnvID:
//=================================================
void sched_kill_env(uint32 envId)
{
	struct Env* ptr_env=NULL;
	int found = 0;
	if (!found)
	{
		LIST_FOREACH(ptr_env, &env_new_queue)
															{
			if(ptr_env->env_id == envId)
			{
				cprintf("killing[%d] %s from the NEW queue...", ptr_env->env_id, ptr_env->prog_name);
				sched_remove_new(ptr_env);
				env_free(ptr_env);
				cprintf("DONE\n");
				found = 1;
				//			return;
			}
															}
	}
	if (!found)
	{
		for (int i = 0 ; i < num_of_ready_queues ; i++)
		{
			if (!LIST_EMPTY(&(env_ready_queues[i])))
			{
				ptr_env=NULL;
				LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
				{
					if(ptr_env->env_id == envId)
					{
						cprintf("killing[%d] %s from the READY queue #%d...", ptr_env->env_id, ptr_env->prog_name, i);
						LIST_REMOVE(&(env_ready_queues[i]), ptr_env);
						env_free(ptr_env);
						cprintf("DONE\n");
						found = 1;
						break;
						//return;
					}
				}
			}
			if (found)
				break;
		}
	}
	if (!found)
	{
		ptr_env=NULL;
		LIST_FOREACH(ptr_env, &env_exit_queue)
		{
			if(ptr_env->env_id == envId)
			{
				cprintf("killing[%d] %s from the EXIT queue...", ptr_env->env_id, ptr_env->prog_name);
				sched_remove_exit(ptr_env);
				env_free(ptr_env);
				cprintf("DONE\n");
				found = 1;
				//return;
			}
		}
	}

	if (!found)
	{
		if (curenv->env_id == envId)
		{
			ptr_env = curenv;
			assert(ptr_env->env_status == ENV_RUNNABLE);
			cprintf("killing a RUNNABLE environment [%d] %s...", ptr_env->env_id, ptr_env->prog_name);
			env_free(ptr_env);
			cprintf("DONE\n");
			found = 1;
			//If it's the curenv, then reset it and reinvoke the scheduler
			//as there's no meaning to return back to a killed env
			//lcr3(K_PHYSICAL_ADDRESS(ptr_page_directory));
			lcr3(phys_page_directory);
			curenv = NULL;
			fos_scheduler();
		}
	}
}

//=================================================
// [12] PRINT ALL Envs from all queues:
//=================================================
void sched_print_all()
{
	struct Env* ptr_env ;
	if (!LIST_EMPTY(&env_new_queue))
	{
		cprintf("\nThe processes in NEW queue are:\n");
		LIST_FOREACH(ptr_env, &env_new_queue)
		{
			cprintf("	[%d] %s\n", ptr_env->env_id, ptr_env->prog_name);
		}
	}
	else
	{
		cprintf("\nNo processes in NEW queue\n");
	}
	cprintf("================================================\n");
	for (int i = 0 ; i < num_of_ready_queues ; i++)
	{
		if (!LIST_EMPTY(&(env_ready_queues[i])))
		{
			cprintf("The processes in READY queue #%d are:\n", i);
			LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
			{
				cprintf("	[%d] %s\n", ptr_env->env_id, ptr_env->prog_name);
			}
		}
		else
		{
			cprintf("No processes in READY queue #%d\n", i);
		}
		cprintf("================================================\n");
	}
	if (!LIST_EMPTY(&env_exit_queue))
	{
		cprintf("The processes in EXIT queue are:\n");
		LIST_FOREACH(ptr_env, &env_exit_queue)
		{
			cprintf("	[%d] %s\n", ptr_env->env_id, ptr_env->prog_name);
		}
	}
	else
	{
		cprintf("No processes in EXIT queue\n");
	}
}

//=================================================
// [13] MOVE ALL NEW Envs into READY Q:
//=================================================
void sched_run_all()
{
	struct Env* ptr_env=NULL;
	LIST_FOREACH(ptr_env, &env_new_queue)
	{
		sched_remove_new(ptr_env);
		sched_insert_ready0(ptr_env);
	}
	/*2015*///if scheduler not run yet, then invoke it!
	if (scheduler_status == SCH_STOPPED)
		fos_scheduler();
}

//=================================================
// [14] KILL ALL Envs in the System:
//=================================================
void sched_kill_all()
{
	struct Env* ptr_env ;
	if (!LIST_EMPTY(&env_new_queue))
	{
		cprintf("\nKILLING the processes in the NEW queue...\n");
		LIST_FOREACH(ptr_env, &env_new_queue)
		{
			cprintf("	killing[%d] %s...", ptr_env->env_id, ptr_env->prog_name);
			sched_remove_new(ptr_env);
			env_free(ptr_env);
			cprintf("DONE\n");
		}
	}
	else
	{
		cprintf("No processes in NEW queue\n");
	}
	cprintf("================================================\n");
	for (int i = 0 ; i < num_of_ready_queues ; i++)
	{
		if (!LIST_EMPTY(&(env_ready_queues[i])))
		{
			cprintf("KILLING the processes in the READY queue #%d...\n", i);
			LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
			{
				cprintf("	killing[%d] %s...", ptr_env->env_id, ptr_env->prog_name);
				LIST_REMOVE(&(env_ready_queues[i]), ptr_env);
				env_free(ptr_env);
				cprintf("DONE\n");
			}
		}
		else
		{
			cprintf("No processes in READY queue #%d\n",i);
		}
		cprintf("================================================\n");
	}

	if (!LIST_EMPTY(&env_exit_queue))
	{
		cprintf("KILLING the processes in the EXIT queue...\n");
		LIST_FOREACH(ptr_env, &env_exit_queue)
		{
			cprintf("	killing[%d] %s...", ptr_env->env_id, ptr_env->prog_name);
			sched_remove_exit(ptr_env);
			env_free(ptr_env);
			cprintf("DONE\n");
		}
	}
	else
	{
		cprintf("No processes in EXIT queue\n");
	}

	//reinvoke the scheduler since there're no env to return back to it
	curenv = NULL;
	fos_scheduler();
}

/*2018*/
//=================================================
// [14] EXIT ALL Ready Envs:
//=================================================
void sched_exit_all_ready_envs()
{
	struct Env* ptr_env=NULL;
	for (int i = 0 ; i < num_of_ready_queues ; i++)
	{
		if (!LIST_EMPTY(&(env_ready_queues[i])))
		{
			ptr_env=NULL;
			LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
			{
				LIST_REMOVE(&(env_ready_queues[i]), ptr_env);
				sched_insert_exit(ptr_env);
			}
		}
	}
}

/*2023*/
/********* for BSD Priority Scheduler *************/

void print_envs(){
	cprintf("\nEnvs: \n");
	ready_queue(p){
		if(!LIST_EMPTY(&env_ready_queues[p]))
			cprintf("\nPriority: %d \n",p);
		ready_env(p,ei){
			cprintf("\n	[%d %s] , env-priority= %d ,env-nice= %d \n",ei->env_id,ei->prog_name,ei->priority,ei->nice_value);
		}
	}
}
int adjust_priority(int x){
	if(x>PRI_MAX)
		return PRI_MAX;
	else if(x<PRI_MIN)
		return PRI_MIN;
	else
		return x ;
}
void update_ready_queue(){
	ready_queue(p){
		struct Env*ei = LIST_FIRST(&env_ready_queues[p]);
		while(ei!=NULL){
			struct Env* next_iteration = LIST_NEXT(ei) ;
			int ep = ei->priority;
			if(ep!=p)
			{
				LIST_REMOVE(&env_ready_queues[p],ei);
				LIST_INSERT_TAIL(&env_ready_queues[ep],ei);
			}
			ei=next_iteration;
		}
	}
}
int priority_without_adjusting(struct Env* e){
	return PRI_MAX-fix_trunc(fix_div(e->recent,fix_int(4)))-2*e->nice_value;
}

int get_value(struct Env* e,int by){
	switch(by){
	case nice:
		return e->nice_value;
	case priority:
		return priority_without_adjusting(e);
	}
	return 0 ;
}
struct Env* merge(struct Env*l,struct Env*r,int by){
	if(r==NULL)
		return l ;
	if(l==NULL)
		return r ;

	struct Env* h=NULL ;
	if(get_value(l,by)>get_value(r,by)){
		h = l ;
		l = r ;
		r = h ;
	}
	h = l ;
	LIST_NEXT(h) = merge(LIST_NEXT(l),r,by);
	LIST_PREV(LIST_NEXT(h)) = h ;
	LIST_PREV(h)=NULL ;
	return h ;
}
struct Env* get_Mid(struct Env* e){
	struct Env*slow = e , *fast = LIST_NEXT(e);
	while(fast!=NULL && LIST_NEXT(fast)!=NULL)
		slow = LIST_NEXT(slow) , fast=LIST_NEXT(LIST_NEXT(fast));
	return slow;
}
struct Env*split(struct Env* head){
	// mid point
	struct Env *mid = get_Mid(head);
	//first of second
	struct Env *b = LIST_NEXT(mid);
	//split
	LIST_NEXT(mid)=NULL ;
	return b ;
}
struct Env* Merge_sort(struct Env* head,int by){
	if(head==NULL || LIST_NEXT(head)==NULL)
		return head ;
	// break list at mid
	struct Env *a = head ,*b = split(head);
	// Recursive sort
	a=Merge_sort(a,by),b = Merge_sort(b,by);
	return merge(a,b,by);
}
void sort_queue(struct Env_Queue *q, int by){
	LIST_FIRST(q) = Merge_sort(LIST_FIRST(q),by);
	LIST_LAST(q)=LIST_FIRST(q) ;
	while(LIST_NEXT(LIST_LAST(q))!=NULL)
		LIST_LAST(q)= LIST_NEXT(LIST_LAST(q)) ;
}
void update_priority_value(struct Env* e){
	int new_priority =priority_without_adjusting(e);
	new_priority = adjust_priority(new_priority);
	e->priority=new_priority;
//	if(old_p==new_priority || e==curenv)
//		return ;
//	LIST_REMOVE(&env_ready_queues[old_p],e);
//	LIST_INSERT_HEAD(&env_ready_queues[new_priority],e);
}
void update_load_avg(uint32 number_of_processes){
	load_avg = fix_add(fix_mul(_59_over_60,load_avg),
			fix_mul(_1_over_60,fix_int(number_of_processes/*number of processes*/)));
	fixed_point_t _2_load_avg = fix_mul(fix_int(2),load_avg) ;
	c_recent = fix_div(_2_load_avg,fix_add(_2_load_avg,fix_int(1)));
}
void update_recent(struct Env* ei){
	ei->recent = fix_add(fix_mul(c_recent,ei->recent),fix_int(ei->nice_value));
//	update_priority_value(ei) ;
}
int64 timer_ticks()
{
	return ticks;
}
int env_get_nice(struct Env* e)
{
	//TODO: [PROJECT'23.MS3 - #3] [2] BSD SCHEDULER - env_get_nice ✅
	return e->nice_value ;
}
void env_set_nice(struct Env* e, int nice_value)
{
	//TODO: [PROJECT'23.MS3 - #3] [2] BSD SCHEDULER - env_set_nice ✅
	e->nice_value = nice_value   ;
	//if(e->env_status!=ENV_NEW)
		update_priority_value(e);
}
int env_get_recent_cpu(struct Env* e)
{
	//TODO: [PROJECT'23.MS3 - #3] [2] BSD SCHEDULER - env_get_recent_cpu ✅
	return fix_round(fix_mul(e->recent,fix_int(100)));
}
int get_load_average()
{
	//TODO: [PROJECT'23.MS3 - #3] [2] BSD SCHEDULER - get_load_average ✅
	return fix_round(fix_mul(load_avg,fix_int(100)));
}
/********* for BSD Priority Scheduler *************/
//==================================================================================//
