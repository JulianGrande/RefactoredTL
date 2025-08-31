/*---------------------------------*
* Julian Grande                    *
* https://github.com/JulianGrande  *
*----------------------------------*/

#ifndef RWTL_H
#define RWTL_H

#define _GNU_SOURCE

/* Possible states threads could be in */

#define RUNNING 1
#define SCHEDULED 2
#define BLOCKED 3

/* Define h files needed */

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

/* define Constants/Macros/Structs/Definitions needed */

typedef unsigned int rwtl_t;

typedef struct ControlBlock{

    rwtl_t ID;
    int status;
    ucontext_t context;
    void* stack;
    int prio;
    long cont_switches;
    double resp_time;
    struct timespec comp_time;
    struct timespec arr_time;
    struct timespec f_time;

} tcb;

typedef struct InternalMutex{

    volatile int flag; 

} internal_mutex;

typedef struct Mutex{

    internal_mutex* mutex; 

} rwtl_mutex;

typedef struct Node{

    tcb* rwt;
    struct Node* next;

} node;

typedef struct Queue{

    int count;
    struct Node* front;
    struct Node* rear;

} queue;

/*Function Declarations */

/* allocate memory to the queue struct */
queue* alloc_queue_new ();

/* make a node and add to the end of the queue */
void enqueue (queue* q, tcb* new_rwt);

/* return entry from top of the queue and deallocate node */
tcb* dequeue (queue* q);

/* check if queue is empty, if empty return 1, if not return 0 */
int isEmpty (queue* q);

/* check if certain thread is in the queue */
int ifExists(rwtl_t curr_ID);

/* find the current running thread in the queue */
node* findRunning(queue* q);

/*remove certain thread from the queue */
void removeFromQ(queue* q, node* target);

/* start timer for optional benchmarks */
void init_timer();

/* create a new thread and add it to the queue */
int rwtl_Create(rwtl_t* new_thread, void *(*function)(void*), void* arg);

/* thread yields remaining time slice */
void rwtl_Yield(int signum);

/* terminate a thread */
void rwtl_Exit();

/*Cleanup remaining allocated structures and data*/
void rwtl_Cleanup();

/* wait for specific thread to terminate */
int rwtl_Join(rwtl_t curr_ID);

/* initilize mutex struct */
void rwtl_init_Mutex(rwtl_mutex* mutex); /* struct st *x = malloc(sizeof *x) */

/* thread grabs the lock to provide mutual exclusion */
int rwtl_Mutex_Lock(rwtl_mutex* mutex);

/* thread releases lock so other threads can do work */
int rwtl_Mutex_Unlock(rwtl_mutex* mutex);

/* deallocate memory for Mutext struct */
void rwtl_Mutex_Destroy(rwtl_mutex* mutex);

/* schedule next thread to run in q */
void schedule();

/* prints time stats */
void print_stats();

#endif