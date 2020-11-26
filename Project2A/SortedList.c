/**
 * NAME: Dhanush Nadella
 * UID: 205150583
 * EMAIL: dnadella787@gmail.com
 * 
 * Relavent Reserach:
 * 
 * strcmp():
 * https://www.tutorialspoint.com/c_standard_library/c_function_strcmp.htm
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>
#include "SortedList.h"


/**
 * Note --> I did not know whether to initialize the sorted list head's
 * next and prev pointer to NULL or to itself. So for my implementation
 * I set it to itself because its supposed to be a circular list anyways.
 * 
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    //if either the element to be added or the header is NULL nothing
    //can be done
    if (list == NULL)
        return;
    
    if (element == NULL)
        return;


    SortedListElement_t *current_node = list->next;
    
    //if there is only head in the list
    if (current_node == list){
        if (opt_yield & INSERT_YIELD)
            sched_yield();

        element->prev = list;
        element->next = list;

        list->prev = element;
        list->next = element;

        return;
    }

    //keep cycling through list till we find a node that is greater than 
    //or equal to the element we want to add
    for (;current_node != list; current_node = current_node->next) {
        int cmp_ret = strcmp(current_node->key, element->key);
        if (cmp_ret >= 0){
            break;
        }
    }

    if (opt_yield & INSERT_YIELD)
        sched_yield();

    element->prev = current_node->prev;
    element->next = current_node;

    current_node->prev->next = element;
    current_node->prev = element;

    return;
}



int SortedList_delete( SortedListElement_t *element){
    if (element == NULL)
        return 1;

    if ((element->next->prev != element) || (element->prev->next != element))
        return 1;

    element->next->prev = element->prev;
    
    if (opt_yield & DELETE_YIELD)
        sched_yield();
    
    element->prev->next = element->next;

    return 0;
}




SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    if (list == NULL)
        return NULL;
    if (key == NULL)
        return NULL;

    for (SortedListElement_t *current_node = list->next; current_node != list; current_node = current_node->next) {
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();

        int cmp_ret = strcmp(key, current_node->key);
        if (cmp_ret == 0) 
            return current_node;
    }

    return NULL;
}



int SortedList_length(SortedList_t *list){
    if (list == NULL)
        return -1;

    int length = 0;
    for (SortedListElement_t *current_node = list->next; current_node != list; current_node = current_node->next) {
        length++;

        if (current_node->prev->next != current_node)
            return -1;
        
        if (current_node->next->prev != current_node)
            return -1;

        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        
    }

    return length;
}