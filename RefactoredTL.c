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

long tot_cntx_switches = 0;
double avg_turn_time = 0;
double avg_resp_time = 0;
double total_turn_time = 0;
double total_resp_time = 0;

/* time keeping */

struct sigaction sa;
struct itimerval timer;
int utime_slice = 5000;
int time_slice = 0;
struct timespec start_time;
struct timespec cur_time;

/* functions */

int rwtl_Create(rwtl_t* new_thread, void *(*function)(void*), void* arg){

    /*initilize main and scheduler contexts, and initilize queue */
    if(total_ran_threads == 0){

        init_timer();
        clock_gettime(CLOCK_MONOTONIC, &start_time);

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

    clock_gettime(CLOCK_MONOTONIC, &cur_time);

    tcb* newThread = (tcb*)malloc(sizeof(tcb));
    newThread->stack = malloc(SIGSTKSZ);
    getcontext(&newThread->context);
    newThread->context.uc_link = 0;
    newThread->context.uc_stack.ss_flags = 0;
    newThread->context.uc_stack.ss_size = SIGSTKSZ;
    newThread->context.uc_stack.ss_sp = newThread->stack;
    makecontext(&newThread->context, (void (*)(void))function, 1, arg);

    newThread->arr_time = cur_time;

    newThread->ID = id_counter++;
    newThread->status = SCHEDULED;
    enqueue(q, newThread);

    if (total_ran_threads == 0) {
		clock_gettime(CLOCK_MONOTONIC, &cur_time);
		newThread->f_time = cur_time;
		double temp_r_time = (newThread->f_time.tv_sec - newThread->arr_time.tv_sec) * 1000 + 
				(newThread->f_time.tv_nsec - newThread->arr_time.tv_nsec) / 1000000;

		newThread->resp_time = temp_r_time;
		total_resp_time += temp_r_time;		
		total_ran_threads++;
		avg_resp_time = total_resp_time / total_ran_threads;
		setitimer(ITIMER_PROF, &timer, NULL);
	}

    return (int)newThread->ID;
}

int rwtl_Yield(){

    node* currentRunning = findRunning(q);

    if(currentRunning == NULL){
        perror("Could not find current running thread\n");
    }

    currentRunning->rwt->cont_switches++;
    currentRunning->rwt->status = SCHEDULED;

    swapcontext(&currentRunning->rwt->context, &sched_context);

    return 0;
}

void rwtl_Exit(){

    node* current = findRunning(q);

    if(current != NULL){

        clock_gettime(CLOCK_MONOTONIC, &cur_time);
		current->rwt->comp_time = cur_time;
		//converted to milliseconds
		double temp_t_time = (current->rwt->comp_time.tv_sec - current->rwt->arr_time.tv_sec) * 1000 + (current->rwt->comp_time.tv_nsec - current->rwt->arr_time.tv_nsec) / 1000000;

		total_turn_time += temp_t_time;
		total_completed_threads++;
		avg_turn_time = total_turn_time / total_completed_threads;
		tot_cntx_switches+= current->rwt->cont_switches;

        removeFromQ(q, current);

    }

    if(q->count == 1){

        tcb* main = dequeue(q);
        free(q);
        free(main->stack);
        free(main);

        setitimer(ITIMER_PROF, 0, NULL);

        setcontext(&mainContext);

    }


    setcontext(&sched_context);
}

int rwtl_Join(rwtl_t curr_ID){

    while(ifExists(curr_ID) == 1){
        rwtl_Yield();
    }

    return 0;
}

void rwtl_init_Mutex(rwtl_mutex* mutex){

    struct Mutex* x = malloc(sizeof(struct Mutex));
    x->flag = 0;

    mutex = x;

}

int rwtl_Mutex_Lock(rwtl_mutex* mutex){

    while(__sync_lock_test_and_set(&mutex->flag, 1) == 1){
        rwtl_Yield();
    }

    return 0;
}

int rwtl_Mutext_Unlock(rwtl_mutex* mutex){

    mutex->flag = 0;

}

void rwtl_Mutex_Destroy(rwtl_mutex* mutex){

    mutex->flag = 0;
    //free(mutex);

}

void init_timer() {
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &rwtl_Yield;
	sigaction(SIGPROF, &sa, NULL);
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;

	timer.it_value.tv_usec = utime_slice;
	timer.it_value.tv_sec = time_slice;

	return;
}

void schedule(){

    tcb* current = dequeue(q);
    current->status = SCHEDULED;
    enqueue(q, current);

    if (q->front->rwt->resp_time == 0 && q->front->rwt->ID != 1) {

			clock_gettime(CLOCK_MONOTONIC, &cur_time); 
			q->front->rwt->f_time = cur_time;

			double temp_r_time = (q->front->rwt->f_time.tv_sec - q->front->rwt->arr_time.tv_sec) * 1000 + 
				(q->front->rwt->f_time.tv_nsec - q->front->rwt->arr_time.tv_nsec) / 1000000;
			q->front->rwt->resp_time = temp_r_time;
			total_resp_time += temp_r_time;
			total_ran_threads++;
			avg_resp_time = total_resp_time / total_ran_threads;
		}

     q->front->rwt->status = RUNNING;
     q->front->rwt->cont_switches++;
     setitimer(ITIMER_PROF, &timer, NULL);
     setcontext(&q->front->rwt->context);

}

void alloc_q (queue* q){

    q = (queue*)malloc(sizeof(queue));

    q->count = 0;
    q->front = NULL;
    q->rear = NULL;

}

int isEmpty (queue* q){

    return (q->rear == NULL);

}

void enqueue (queue* q, tcb* new_rwt){

    node* temp = (node*)malloc(sizeof(node));
    temp->rwt = new_rwt;
    temp->next = NULL;

    if(!isEmpty(q)){

        q->rear->next = temp;
        q->rear = temp;

    }
    else{

        q->front = q->rear = temp;

    }

    q->count++;
}

tcb* dequeue (queue* q){

    node* temp;
    tcb* tempRWT = q->front->rwt;
    temp = q->front;
    q->front = q->front->next;
    q->count--;
    free(temp);
    return tempRWT;

}

int ifExists(rwtl_t curr_ID){

    node* ptr = q->front;

    while(ptr != NULL){

        if(ptr->rwt->ID == curr_ID){
            return 1;
        }

        ptr = ptr->next;
    }

    return 0;
}

node* findRunning(queue* q){

    node* ptr = q->front;

    while(ptr != NULL){

        if(ptr->rwt->status == RUNNING){
            return ptr;
        }

        ptr = ptr->next;
    }

    return NULL;
}

void removeFromQ(queue* q, node* target){

    node* ptr = q->front;
	node* temp = ptr;

	if (ptr->rwt->ID == target->rwt->ID) {

		q->front = ptr->next;
		ptr->rwt->status = BLOCKED;
		free(ptr->rwt->stack);
		free(ptr->rwt);
		free(ptr);
		ptr = NULL;
		q->count--;
		return;
	}
	ptr = ptr->next;

	while (ptr != NULL) {
		if (ptr->rwt->ID == target->rwt->ID) {

			temp->next = ptr->next;
			ptr->rwt->status = BLOCKED;
			free(ptr->rwt->stack);
			free(ptr->rwt);
			free(ptr);
			ptr = NULL;
			q->count--;
			return;
		}
		temp = ptr;
		ptr = ptr->next;
	}

}

void print_stats(){

    printf("Total Context Switches: %ld \n", tot_cntx_switches);
    printf("Average Turnaround Time: %lf \n", avg_turn_time);
    printf("Average Response Time: %lf", avg_resp_time);

}