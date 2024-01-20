# User Level Threading Library

## Overview

This library built in C utilizes ucontext to create lightweight threads in the user space. It is designed to have a similar interface to the pThread library, however it features a FIFO scheduling mechanism utilizing a queue data structure with similar evicition policies that are enforced by timers, signal handling, and system calls.

Threads are represented as a node data structure known as the thread control block wherein the necessary information and states are stored such as id, status, context/execution state, allocated stack space, and various time keeping statistics. Each node has a control block and a pointer to another node, these nodes are then used to schedule which threads run until the function call finishes or until the designated time slice is finished.

The library also supports mutual exclusion locks or mutex; implemented using a structure that has a flag with initial value of 0, the lock utilizes "__sync_lock_test_and_set()" in order to atomically update the flag value to 1 and test whether or not the lock has been released, ensuring complete mutual exclusion between threads.

## API

Thread
  * rwtl_Create() - On first occurence, performs intialization of main context, scheduler context, and allocation for both context's stack, creates a thread and initializes node and thread control block
  * rwtl_Yield()  - Searches queue for current running thread and returns pointer to it, updating its control block status and swapping into the scheduler context
  * rwtl_Exit()   - Finds current thread in scheduler, updates average time statistics from the information in the control block, and freeing any allocated memory
  * rwtl_Join()   - Utilized by the main context, find the current thread by its thread ID, and wait until it finishes by calling rwtl_Exit()

Mutex
  * rwtl_init_Mutex()    - Allocate a simple rwtl_mutex struct which has an integer that we initilze to 0
  * rwtl_Mutex_Lock()    - Uses "__sync_lock_test_and_set()" in a while loop to ensure mutual exclusion to the user
  * rwtl_Mutext_Unlock() - Sets the mutex struct flag to 0, signifying that the lock is available for another thread 
  * rwtl_Mutex_Destroy() - Set the mutext struct flag to 0 and free allocated space

Misc
  * Various helper functions
  * init_timer() - Timer is set whenever a thread runs, when the timer runs out it interrupts the thread, and calls rwtl_Yield()
  * schedule()   - FIFO based linked list queue implementation, a new thread is added to the back of the list upon creation, if a thread runs for the first time, global time variables are updated, and upon every dequeue the number of context switches is updated

## Helpful Links

* [getcontext](https://man7.org/linux/man-pages/man3/getcontext.3.html)
* [makecontext](https://man7.org/linux/man-pages/man3/makecontext.3.html)
* [swapcontext](https://linux.die.net/man/3/swapcontext)
* [signal](https://man7.org/linux/man-pages/man7/signal.7.html)
* [sys/time](https://man7.org/linux/man-pages/man0/sys_time.h.0p.html)


- - -
2024 Julian Grande
