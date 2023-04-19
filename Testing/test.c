#include <stdio.h>
#include <unistd.h>
#include "../RefactoredTL.h"

rwtl_t* threads;
rwtl_mutex mt;
int* counter;
int sum;

void external_cal(void* arg){

    int n = *((int*) arg);

    for(int i = 0; i < 2; i++){
        rwtl_Mutex_Lock(&mt);
        sum += n;
        rwtl_Mutext_Unlock(&mt);
    }

    rwtl_Exit();
}

int main(int argc, char** argv){

    threads = (rwtl_t*)malloc(2*sizeof(rwtl_t));
    counter = (int*)malloc(2*sizeof(int));

    rwtl_init_Mutex(&mt);

    for(int k = 0; k < 2; k++){
        counter[k] = k+1;
    }

    for(int i = 0; i < 2; i++){
        rwtl_Create(&threads[i], &external_cal, &counter[i]);
    }

    for(int j = 0; j < 2; j++){
        rwtl_Join(threads[j]);
    }

    rwtl_Mutex_Destroy(&mt);

    free(threads);
    free(counter);

    printf("Total Sum is: %d\n", sum);
    print_stats();

    return 0;
}