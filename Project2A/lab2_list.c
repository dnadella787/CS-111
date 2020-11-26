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
 * 
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
SortedList_t *list_head;


int num_iterations = 1;

int mutex_flag = 0;
int spin_flag = 0;

pthread_mutex_t restrict_mutex;
int lock = 0;


void *start_routine(void *element){
    SortedListElement_t *start_element = (SortedListElement_t *)element;
    for (int i = 0; i < num_iterations; i++){
        if (mutex_flag == 1){
            pthread_mutex_lock(&restrict_mutex);
        }
        else if (spin_flag == 1){
            while (__sync_lock_test_and_set(&lock, 1));
        }

        SortedList_insert(list_head, (&start_element[i]));
        
        if (mutex_flag == 1){
            pthread_mutex_unlock(&restrict_mutex);
        }
        else if (spin_flag == 1) {
            __sync_lock_release(&lock);
        }
    }

    if (mutex_flag == 1){
        pthread_mutex_lock(&restrict_mutex);
    }
    else if (spin_flag == 1){
        while (__sync_lock_test_and_set(&lock, 1));
    }

    int length = SortedList_length(list_head);
    if (length == -1){
        fprintf(stderr, "List corrupted, error getting length of SortedList.\n");
        exit(2);
    }

    if (mutex_flag == 1){
        pthread_mutex_unlock(&restrict_mutex);
    }
    else if (spin_flag == 1) {
        __sync_lock_release(&lock);
    }

    int delete_return;
    SortedListElement_t *lookup_return;
    for (int i = 0; i < num_iterations; i++){
        if (mutex_flag == 1){
            pthread_mutex_lock(&restrict_mutex);
        }
        else if (spin_flag == 1){
            while (__sync_lock_test_and_set(&lock, 1));
        }

        lookup_return = SortedList_lookup(list_head, start_element[i].key);
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
            pthread_mutex_unlock(&restrict_mutex);
        }
        else if (spin_flag == 1) {
            __sync_lock_release(&lock);
        }
    }

    return NULL;
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


            default: //? is returned because an incorrect flag was used
                fprintf(stderr, "Incorrect flag usage. Allowed flags: --yield=[idl] --threads=# --iterations=#\n");
                exit(1);
            }
    }

    //create lock for the pthread_mutex sync option (m)
    if (mutex_flag == 1){
        int pthreadinit_retval = pthread_mutex_init(&restrict_mutex, NULL);
        if (pthreadinit_retval != 0){
            fprintf(stderr, "Error initializing pthread_mutex_t.\n");
            exit(2);
        }
    }

    //allocate memory for list head 
    list_head = (SortedList_t *)malloc(sizeof(SortedList_t));
    if (list_head == NULL){
        fprintf(stderr, "Error allocating memory for list head.\n");
        exit(2);
    }

    //head is defined by NULL key and its previous and next pointer to itself at starting
    list_head->key = NULL;
    list_head->prev = list_head;
    list_head->next = list_head;

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
        free(temp_key);
    }


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

    int length_of_list = SortedList_length(list_head);
    if (length_of_list == -1){
        fprintf(stderr, "List corrupted, error getting length of SortedList at end.\n");
        exit(2);
    }
    else if (length_of_list != 0){
        fprintf(stderr, "List corrupted, length is not 0 at the end.\n");
        exit(2);
    }

    
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
    int num_list = 1;
    long long total_operations = num_threads * num_iterations * 3;
    long long total_runtime = (1000000000 * (ending_time.tv_sec - starting_time.tv_sec)) + (ending_time.tv_nsec - starting_time.tv_nsec);
    long long avg_time = total_runtime / total_operations;
    
    printf("%s,%d,%d,%d,%lld,%lld,%lld\n", test_name,
                                        num_threads, 
                                        num_iterations, 
                                        num_list,
                                        total_operations, 
                                        total_runtime, 
                                        avg_time);

    free(list_head);
    free(list);

    exit(0);
}

