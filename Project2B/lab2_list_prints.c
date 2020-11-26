/**
 * NAME: Dhanush Nadella
 * UID: 205150584
 * EMAIL: dnadella787@gmail.com 
 * 
 * Relavent Research:
 * 
 * Random Letter Generation for Keys:
 * https://stackoverflow.com/questions/19724346/generate-random-characters-in-c
 * 
 * getting return values using pthread_join:
 * https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/ThreadArgs.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "SortedList.h"

int opt_yield = 0;
SortedList_t *list_head_array;

int num_iterations = 1;
int num_list = 1;

int mutex_flag = 0;
int spin_flag = 0;

pthread_mutex_t *restrict_mutex_array = NULL;
int *spin_lock_array = NULL;


void *start_routine(void *element){
    long long lock_acq_time = 0;
    struct timespec starting_time, ending_time;
    int hash_index;
    SortedListElement_t *start_element = (SortedListElement_t *)element;
    for (int i = 0; i < num_iterations; i++){
        hash_index = ((int)start_element[i].key[0] + ((int)start_element[i].key[1] * 26)) % num_list;
        if (mutex_flag == 1){
            int time_retval = clock_gettime(CLOCK_MONOTONIC, &starting_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at start.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            pthread_mutex_lock(&restrict_mutex_array[hash_index]);
            time_retval = clock_gettime(CLOCK_MONOTONIC, &ending_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at end.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            lock_acq_time += (1000000000 * (ending_time.tv_sec - starting_time.tv_sec)) + (ending_time.tv_nsec - starting_time.tv_nsec);
        }
        else if (spin_flag == 1){
            int time_retval = clock_gettime(CLOCK_MONOTONIC, &starting_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at start.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            while (__sync_lock_test_and_set(&spin_lock_array[hash_index], 1));
            time_retval = clock_gettime(CLOCK_MONOTONIC, &ending_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at end.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            lock_acq_time += (1000000000 * (ending_time.tv_sec - starting_time.tv_sec)) + (ending_time.tv_nsec - starting_time.tv_nsec);
        }

        SortedList_insert((list_head_array + hash_index), (start_element + i));
        
        if (mutex_flag == 1){
            pthread_mutex_unlock(&restrict_mutex_array[hash_index]);
        }
        else if (spin_flag == 1) {
            __sync_lock_release(&spin_lock_array[hash_index]);
        }
    }

    long total_list_length = 0;
    long per_list_length = 0;
    for (int i = 0; i < num_list; i++)
    {
        if (mutex_flag == 1){
            int time_retval = clock_gettime(CLOCK_MONOTONIC, &starting_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at start.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            pthread_mutex_lock(&restrict_mutex_array[i]);
            time_retval = clock_gettime(CLOCK_MONOTONIC, &ending_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at end.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            lock_acq_time += (1000000000 * (ending_time.tv_sec - starting_time.tv_sec)) + (ending_time.tv_nsec - starting_time.tv_nsec);
        }
        else if (spin_flag == 1){
            int time_retval = clock_gettime(CLOCK_MONOTONIC, &starting_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at start.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            while (__sync_lock_test_and_set(&spin_lock_array[i], 1));
            time_retval = clock_gettime(CLOCK_MONOTONIC, &ending_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at end.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            lock_acq_time += (1000000000 * (ending_time.tv_sec - starting_time.tv_sec)) + (ending_time.tv_nsec - starting_time.tv_nsec);
        }
        per_list_length = SortedList_length((list_head_array + i));
        if (per_list_length == -1){
            fprintf(stderr, "List corrupted, error getting length of SortedList.\n");
            exit(2);
        }
        total_list_length += per_list_length;
        if (mutex_flag == 1){
            pthread_mutex_unlock(&restrict_mutex_array[i]);
        }
        else if (spin_flag == 1) {
            __sync_lock_release(&spin_lock_array[i]);
        }
    }



    int delete_return;
    SortedListElement_t *lookup_return;
    for (int i = 0; i < num_iterations; i++){
        hash_index = ((int)start_element[i].key[0] + ((int)start_element[i].key[1] * 26)) % num_list;
        if (mutex_flag == 1){
            int time_retval = clock_gettime(CLOCK_MONOTONIC, &starting_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at start.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            pthread_mutex_lock(&restrict_mutex_array[hash_index]);
            time_retval = clock_gettime(CLOCK_MONOTONIC, &ending_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at end.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            lock_acq_time += (1000000000 * (ending_time.tv_sec - starting_time.tv_sec)) + (ending_time.tv_nsec - starting_time.tv_nsec);
        }
        else if (spin_flag == 1){
            int time_retval = clock_gettime(CLOCK_MONOTONIC, &starting_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at start.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            while (__sync_lock_test_and_set(&spin_lock_array[hash_index], 1));
            time_retval = clock_gettime(CLOCK_MONOTONIC, &ending_time);
            if (time_retval < 0){
                fprintf(stderr, "Clock_gettime failed at end.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            lock_acq_time += (1000000000 * (ending_time.tv_sec - starting_time.tv_sec)) + (ending_time.tv_nsec - starting_time.tv_nsec);
        }

        lookup_return = SortedList_lookup((list_head_array + hash_index), (start_element + i)->key);
        if (lookup_return == NULL){
            fprintf(stderr, "List corrupted, element that should be there not found.\n");
            exit(2);
        }
        delete_return = SortedList_delete(lookup_return);
        if (delete_return == 1){
            fprintf(stderr, "List corrupted, element found but couldn't be deleted.\n");
            exit(2);
        }

        if (mutex_flag == 1){
            pthread_mutex_unlock(&restrict_mutex_array[hash_index]);
        }
        else if (spin_flag == 1) {
            __sync_lock_release(&spin_lock_array[hash_index]);
        }
    }

    pthread_exit((void *)lock_acq_time);
}



void segfault_handler(){
    fprintf(stderr, "List corrupted, segmentation fault ocurred.\n");
    exit(2);
}



int main(int argc, char** argv){
    signal(SIGSEGV, segfault_handler);

    char test_name[50] = "list";

    int num_threads = 1;

    int yield_flag = 0;
    char* yield_arguments;
    int insert_flag = 0;
    int delete_flag = 0;
    int lookup_flag = 0;

    int sync_flag = 0;

    int c;

    while(1){
        static struct option long_options[] =
            {
                {"threads", required_argument, 0, 't'},
                {"iterations", required_argument, 0, 'i'},
                {"yield", required_argument, 0, 'y'},
                {"sync", required_argument, 0, 's'},
                {"lists", required_argument, 0, 'l'},
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
                yield_arguments = optarg;
                yield_flag = 1;
                for (int i = 0; i < (int)strlen(yield_arguments);i++){
                    if (yield_arguments[i] == 'i'){
                        opt_yield = opt_yield | INSERT_YIELD;
                        insert_flag = 1;
                    }
                    else if (yield_arguments[i] == 'd'){
                        opt_yield = opt_yield | DELETE_YIELD;
                        delete_flag = 1;
                    }
                    else if (yield_arguments[i] == 'l'){
                        opt_yield = opt_yield | LOOKUP_YIELD;
                        lookup_flag = 1;
                    }
                    else {
                        fprintf(stderr, "Incorrect flag usage for --yield, only [idl] allowed.\n");
                        exit(1);
                    }
                }
                break;

            case 's':
                sync_flag = 1;
                if (optarg[0] == 'm') {
                    mutex_flag = 1;
                }
                else if (optarg[0] == 's'){
                    spin_flag = 1;
                }
                else {
                    fprintf(stderr, "Incorrect flag usage for --sync, only m or s allowed.\n");
                    exit(1);
                }
                break;

            case 'l':
                num_list = atoi(optarg);
                break;

            default: //? is returned because an incorrect flag was used
                fprintf(stderr, "Incorrect flag usage. Allowed flags: --yield=[idl] --threads=# --iterations=#\n");
                exit(1);
            }
    }

    printf("get opt done\n");
    //create lock for the pthread_mutex sync option (m)
    if (mutex_flag == 1)
    {
        restrict_mutex_array = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * num_list);
        if (restrict_mutex_array == NULL){
            fprintf(stderr, "Error allocating memory for restrict_mutex_array.\n");
            exit(2);
        }
        int pthreadinit_retval;
        for (int i = 0; i < num_list; i++){
            pthreadinit_retval = pthread_mutex_init(&restrict_mutex_array[i], NULL);
            if (pthreadinit_retval != 0){
                fprintf(stderr, "Error initializing pthread_mutex_t.\n");
                exit(2);
            }
        }
    }
    else if (spin_flag == 1)
    {
        spin_lock_array = (int *)calloc(num_list, sizeof(int));
        if (spin_lock_array == NULL){
            fprintf(stderr, "Error allocating memory for spin_lock_array.\n");
            exit(2);
        }
    }
    printf("restrict_mutex_array and spin_lock_array allocated\n");
    //allocate memory for --lists=# heads
    list_head_array = (SortedList_t *)malloc(num_list * sizeof(SortedList_t));
    if (list_head_array == NULL){
        fprintf(stderr, "Error allocating memory for list head.\n");
        exit(2);
    }

    //head is defined by NULL key and its previous and next pointer to itself at starting
    for (int i = 0; i < num_list; i++){
        list_head_array[i].key = NULL;
        list_head_array[i].prev = &list_head_array[i];
        list_head_array[i].next = &list_head_array[i];
    }
    printf("list_head_arry allocated and initialized\n");
    //list of elements
    SortedListElement_t *list = (SortedListElement_t *)malloc(sizeof(SortedListElement_t) * (num_threads * num_iterations));
    if (list == NULL){
        fprintf(stderr, "Error allocating memory for empty list.\n");
        exit(2);
    }

    //initialize each element in the list with a random 2 letter lower case key
    for (int i = 0; i < (num_threads * num_iterations); i++){
        char *temp_key = (char *)malloc(sizeof(char) * 3);
        for (int j = 0; j < 2; j++){
            temp_key[j] =  'a' + (rand() & 26);
        }
        temp_key[2] = '\0';
        list[i].key = temp_key;
    }

    printf("list of elements allocated and have keys\n");

    //array of pthreads for pthread_create
    pthread_t pthread_array[num_threads];

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
        create_retval = pthread_create(&(pthread_array[i]), NULL, start_routine, (void *)(list + (num_iterations * i)));
        if (create_retval != 0){
            fprintf(stderr, "Pthread_create error.\n");
            exit(2);
        }
    }
    printf("pthreads all created and time gotten\n");
    //wait for the termination of each thread
    long long total_lock_acq_time = 0;
    void *thread_lock_acq_time;
    int join_retval;
    for (int i = 0; i < num_threads; i++) {
        join_retval = pthread_join(pthread_array[i], &thread_lock_acq_time);
        if (join_retval != 0){
            fprintf(stderr, "Pthread_join error.\n");
            exit(2);
        }
        total_lock_acq_time += (long long)thread_lock_acq_time;
    }

    //get ending time
    time_retval = clock_gettime(CLOCK_MONOTONIC, &ending_time);
    if (time_retval < 0){
        fprintf(stderr, "Clock_gettime failed at end.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    printf("pthreads all joined back and time gotten\n");

    int per_list_length = 0;
    for (int i = 0; i < num_list; i++)
    {
        per_list_length = SortedList_length((list_head_array + i));
        if (per_list_length == -1){
            fprintf(stderr, "List corrupted, error getting length of a sub list at end.\n");
            exit(2);
        }
        else if (per_list_length != 0){
            fprintf(stderr, "List corrupted, all sublists do not have length 0.\n");
            exit(2);
        }
    }

    printf("list length all checked in main\n");

    if (yield_flag == 0){
        strcat(test_name, "-none");
    }
    else{
        strcat(test_name, "-");
        if (insert_flag == 1){
            strcat(test_name, "i");
        }
        if (delete_flag == 1){
            strcat(test_name, "d");
        }
        if (lookup_flag == 1){
            strcat(test_name, "l");
        }
    }

    if (sync_flag == 0){
        strcat(test_name,"-none");
    }
    else{
        strcat(test_name, "-");
        if (mutex_flag == 1){
            strcat(test_name, "m");
        }
        else if (spin_flag == 1){
            strcat(test_name, "s");
        }
    }


    //calculations for testing and .csv
    long long total_operations = num_threads * num_iterations * 3;
    long long total_runtime = (1000000000 * (ending_time.tv_sec - starting_time.tv_sec)) + (ending_time.tv_nsec - starting_time.tv_nsec);
    long long avg_time = total_runtime / total_operations;
    long long avg_wait_for_lock = total_lock_acq_time / total_operations;

    printf("%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", test_name,
                                        num_threads, 
                                        num_iterations, 
                                        num_list,
                                        total_operations, 
                                        total_runtime, 
                                        avg_time,
                                        avg_wait_for_lock);

    printf("test name done and calculations done and printed\n");


    if (mutex_flag == 1){
        int destroy_ret;
        for (int i = 0; i < num_list; i++){
            destroy_ret = pthread_mutex_destroy(&restrict_mutex_array[i]);
            if (destroy_ret != 0){
                fprintf(stderr, "Error with pthread_mutex_destroy.\n");
                exit(2);
            }
        }
        free(restrict_mutex_array);
        printf("pthread_mutex destroyed\n");
    }
    else if (spin_flag == 1){
        free(spin_lock_array);
    }

    free(list_head_array);
    for (int i = 0; i < (num_threads * num_iterations); i++){
        free((void *)(list + i)->key);
    }
    free(list);

    printf("free statements done\n");
    exit(0);
}

