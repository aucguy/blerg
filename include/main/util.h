#ifndef UTIL_H_
#define UTIL_H_

/**
 * Copies the given string
 */
const char* newStr(const char* src);

/**
 * A forward linked list
 */
typedef struct List_ {
    //the data of this node
    void* head;
    //the rest of the list
    struct List_* tail;
} List;

/**
 * Returns a new list with an item prepended.
 */
List* consList(void* head, List* tail);

/**
 * Frees the list. The 'func' argument is called for each element.
 */
void destroyList(List* list, void(*func)(void*));

#endif /* UTIL_H_ */
