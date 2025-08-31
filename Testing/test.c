#include <stdio.h>
#include <unistd.h>
#include "../RefactoredTL.h"

#define NUM_THREADS 10

rwtl_t* threads;
rwtl_mutex mt;
int* counter;
int sum;

void external_cal(void* arg){

    int n = *((int*) arg);

    for(int i = 0; i < 2; i++){
        rwtl_Mutex_Lock(&mt);
        sum += n;
        rwtl_Mutex_Unlock(&mt);
    }

    rwtl_Exit();
}

int main(int argc, char** argv){

    threads = (rwtl_t*)malloc(NUM_THREADS*sizeof(rwtl_t));
    counter = (int*)malloc(NUM_THREADS*sizeof(int));

    rwtl_init_Mutex(&mt);

    for(int k = 0; k < NUM_THREADS; k++){
        counter[k] = k+1;
    }

    for(int i = 0; i < NUM_THREADS; i++){
        rwtl_Create(&threads[i], &external_cal, &counter[i]);
        printf("Created thread: %d\n", i);
    }

    for(int j = 0; j < NUM_THREADS; j++){
        rwtl_Join(threads[j]);
        printf("Joined thread: %d\n", j);
    }

    rwtl_Mutex_Destroy(&mt);
    rwtl_Cleanup();

    free(threads);
    free(counter);

    printf("Total Sum is: %d\n", sum);
    print_stats();

    return 0;
}