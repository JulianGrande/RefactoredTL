/*---------------------------------*
* Julian Grande                    *
* https://github.com/JulianGrande  *
*----------------------------------*/

#include "RefactoredTL.h"

/* global values/variables */

int id_counter = 1; //num of active threads
int total_completed_threads = 0;
int total_ran_threads = 0;
static ucontext_t mainContext;
ucontext_t context, sched_context;
void* sched_stack;
queue* q;

/* functions */

int rwtl_Create(rwtl_t* new_thread, void *(*function)(void*), void* arg){

    /*initilize main and scheduler contexts, and initilize queue */
    if(total_ran_threads == 0){

        tcb* main = (tcb*)malloc(sizeof(tcb));
        main->stack = malloc(SIGSTKSZ);
        getcontext(&main->context);
        main->context.uc_link = 0;
        main->context.uc_stack.ss_flags = 0;
        main->context.uc_stack.ss_size = SIGSTKSZ;
        main->context.uc_stack.ss_sp = main->stack;

        main->ID = id_counter++;
        main->status = RUNNING;

        sched_stack = malloc(SIGSTKSZ);
        getcontext(&sched_context);
        sched_context.uc_link = 0;
        sched_context.uc_stack.ss_flags = 0;
        sched_context.uc_stack.ss_size = SIGSTKSZ;
        sched_context.uc_stack.ss_sp = sched_stack;
        makecontext(&sched_context, (void*)&schedule, 0);
        
        alloc_q(q);
        enqueue(q, main);

    }

    tcb* newThread = (tcb*)malloc(sizeof(tcb));
    newThread->stack = malloc(SIGSTKSZ);
    getcontext(&newThread->context);
    newThread->context.uc_link = 0;
    newThread->context.uc_stack.ss_flags = 0;
    newThread->context.uc_stack.ss_size = SIGSTKSZ;
    newThread->context.uc_stack.ss_sp = newThread->stack;
    makecontext(&newThread->context, (void (*)(void))function, 1, arg);

    newThread->ID = id_counter++;
    newThread->status = SCHEDULED;
    enqueue(q, newThread);

    return (int)newThread->ID;
}

int rwtl_Yield(){


    return 0;
}

void alloc_q (queue* q){

    q = (queue*)malloc(sizeof(queue));

    q->count = 0;
    q->front = NULL;
    q->rear = NULL;

}