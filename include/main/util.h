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
 * Returns true if all items in the lists satisfy the predicate. For each
 * index between 0 and the length of the lists, the item listA[i] and listB[i]
 * are passed to the predicate. The function will only return true if the
 * predicate returns true for all invocations. Also, the function will return
 * false if the lists are of different lengths.
 */
int allList(List* listA, List* listB, int(*predicate)(void*, void*));

/**
 * Returns true if all the items in the list satisfy the predicate. For each
 * index between 0 and the length of this list, the item list[i] is passed to
 * the predicate. The function will only return true if the predicate returns
 * true for all invocations.
 */
int allList(List* list, int(*predicate)(void*));

/**
 * Frees the list. The 'func' argument is called for each element.
 */
void destroyList(List* list, void(*func)(void*));

/**
 * Frees the list structure, but not its contents.
 */
void destroyShallowList(List* list);

/**
 * Returns the last item in the list.
 */
void* lastList(List* list);

/**
 * Returns a new list with the contents of before prepended to after, but in
 * reverse order. For example, prependReverseList([1, 2, 3], [4, 5, 6]) ==
 * [3, 2, 1, 4, 5, 6].
 */
List* prependReverseList(List* before, List* after);

/**
 * Returns the list in reverse. For example reverseList([1, 2, 3]) == [3, 2, 1]
 */
List* reverseList(List* list);

#endif /* UTIL_H_ */
