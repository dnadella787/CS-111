/**
 * NAME: Dhanush Nadella 
 * UID: 205150583
 * EMAIL: dnadella787@gmail.com
 * 
 * relavent research:
 * 
 * pthread:
 * https://man7.org/linux/man-pages/man3/pthread_create.3.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_join.html
 * 
 * clock_gettime:
 * https://man7.org/linux/man-pages/man2/clock_gettime.2.html
 * https://en.cppreference.com/w/c/chrono/timespec
 * 
 * concatenating strings in C:
 * https://www.techonthenet.com/c_language/standard_library_functions/string_h/strcat.php
 * 
 * (2) vs (3), did not know this
 * https://unix.stackexchange.com/questions/3586/what-do-the-numbers-in-a-man-page-mean#:~:text=Man%20pages%20are%20organized%20in,is%20for%20library%20functions%20etc.
 * 
 * spinlock:
 * https://attractivechaos.wordpress.com/2011/10/06/multi-threaded-programming-efficiency-of-locking/
 * 
 * __sync_val_compare_and_swap:
 * https://www.ibm.com/support/knowledgecenter/SS2LWA_12.1.0/com.ibm.xlcpp121.bg.doc/compiler_ref/bif_gcc_atomic_val_comp_swap.html
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <sched.h>


int num_iterations = 1;
int opt_yield = 0;

int pthread_mutex_flag = 0;
int spinlock_flag = 0;
int compswap_flag = 0;

int lock = 0;

pthread_mutex_t restrict_mutex;


void add(long long *pointer, long long value){
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}


void pthread_add(long long *pointer, long long value){
    pthread_mutex_lock(&restrict_mutex);
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
    pthread_mutex_unlock(&restrict_mutex);
}


void spinlock_add(long long *pointer, long long value){
    while (__sync_lock_test_and_set(&lock, 1));
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
    __sync_lock_release(&lock);
}


void compswap_add(long long *pointer, long long value){
    long long curr_counter, updated_counter;
    do {
        curr_counter = *pointer;
        updated_counter = curr_counter + value;
        if (opt_yield)
            sched_yield();
    } while (__sync_val_compare_and_swap(pointer, curr_counter, updated_counter) != curr_counter);
}

void *start_routine(void *void_counter){
    long long *counter = (long long *)void_counter;

    for (int i = 0; i < num_iterations; i++){
        if (pthread_mutex_flag == 1){
            pthread_add(counter, 1);
        }
        else if (spinlock_flag == 1){
            spinlock_add(counter, 1);
        }
        else if (compswap_flag == 1){
            compswap_add(counter, 1);
        }
        else {
            add(counter, 1);
        }
    }

    for (int i = 0; i < num_iterations; i++){
        if (pthread_mutex_flag == 1){
            pthread_add(counter, -1);
        }
        else if (spinlock_flag == 1){
            spinlock_add(counter, -1);
        }
        else if (compswap_flag == 1){
            compswap_add(counter, -1);
        }
        else {
            add(counter, -1);
        }
    }
    return NULL;
}


int main(int argc, char** argv){
    int synchronization = 0;

    char test_name[50] = "add";

    int num_threads = 1;

    long long counter = 0;

    int c;

    while(1){
        static struct option long_options[] =
            {
                {"threads", required_argument, 0, 't'},
                {"iterations", required_argument, 0, 'i'},
                {"yield", no_argument, 0, 'y'},
                {"sync", required_argument, 0, 's'},
                {0, 0, 0, 0}
            };

        int option_index = 0;

        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1) //end reached
            break;

        switch(c){
            case 't':  //--threads flag
                num_threads = atoi(optarg);
                break;
                
            case 'i': //--iterations flag
                num_iterations = atoi(optarg);
                break;

            case 'y': //--yield flag
                opt_yield = 1;
                break;

            case 's': //--sync flag
                if (optarg[0] == 'm'){
                    pthread_mutex_flag = 1;
                }
                else if (optarg[0] == 's'){
                    spinlock_flag = 1;
                }
                else if (optarg[0] == 'c'){
                    compswap_flag = 1;
                }
                else {
                    fprintf(stderr, "Incorrect arg for --sync, only m, s, and c allowed.\n");
                    exit(1);
                }
                synchronization = 1;
                break;

            default: //? is returned because an incorrect flag was used
                fprintf(stderr, "Incorrect flag usage. Allowed flags: --yield --sync=[m,c,s] --threads=# --iterations=#\n");
                exit(1);
            }
    }

    //array of pthreads for pthread_create
    pthread_t pthread_array[num_threads];

    //create lock for the pthread_mutex sync option (m)
    if (pthread_mutex_flag == 1){
        int pthreadinit_retval = pthread_mutex_init(&restrict_mutex, NULL);
        if (pthreadinit_retval != 0){
            fprintf(stderr, "Error initializing pthread_mutex_t.\n");
            exit(2);
        }
    }

    //note starting time
    struct timespec starting_time, ending_time;
    int time_retval = clock_gettime(CLOCK_MONOTONIC, &starting_time);
    if (time_retval < 0){
        fprintf(stderr, "Clock_gettime failed at start.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    //create the threads
    int create_retval;
    for (int i = 0; i < num_threads; i++) {
        create_retval = pthread_create(&(pthread_array[i]), NULL, start_routine, (void *)(&counter));
        if (create_retval != 0){
            fprintf(stderr, "Pthread_create error.\n");
            exit(2);
        }
    }

    //wait for the termination of each thread
    int join_retval;
    for (int i = 0; i < num_threads; i++) {
        join_retval = pthread_join(pthread_array[i], NULL);
        if (join_retval != 0){
            fprintf(stderr, "Pthread_join error.\n");
            exit(2);
        }
    }

    //get ending time
    time_retval = clock_gettime(CLOCK_MONOTONIC, &ending_time);
    if (time_retval < 0){
        fprintf(stderr, "Clock_gettime failed at end.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }


    //set proper testname
    if (opt_yield == 1)
        strcat(test_name, "-yield");
    
    if (pthread_mutex_flag == 1)
        strcat(test_name, "-m");

    else if (spinlock_flag == 1)
        strcat(test_name, "-s");
    
    else if (compswap_flag == 1)
        strcat(test_name, "-c");
    
    if (synchronization == 0)
        strcat(test_name, "-none");
    

    //calculations for testing and .csv
    long long total_operations = num_threads * num_iterations * 2;
    long long total_runtime = (1000000000 * (ending_time.tv_sec - starting_time.tv_sec)) + (ending_time.tv_nsec - starting_time.tv_nsec);
    long long avg_time = total_runtime / total_operations;
    
    printf("%s,%d,%d,%lld,%lld,%lld,%lld\n", test_name, num_threads, num_iterations, total_operations, total_runtime, avg_time, counter);

    exit(0);
}