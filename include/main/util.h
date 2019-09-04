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
 * Returns true if all items in the lists satisify the predicate. For each
 * index between 0 and the length of the lists, the item listA[i] and listB[i]
 * are passed to the predicate. The function will only return true if predicate
 * returns true for all invocations. Also, the function will return false if
 * the lists are of different lengths.
 */
int allList(List* listA, List* listB, int(*predicate)(void*, void*));

/**
 * Frees the list. The 'func' argument is called for each element.
 */
void destroyList(List* list, void(*func)(void*));

#endif /* UTIL_H_ */
