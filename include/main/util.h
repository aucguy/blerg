#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>

/**
 * Copies the given string
 */
char* newStr(const char* src);

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
uint8_t allList2(List* listA, List* listB, uint8_t(*predicate)(void*, void*));

/**
 * Returns true if all the items in the list satisfy the predicate. For each
 * index between 0 and the length of this list, the item list[i] is passed to
 * the predicate. The function will only return true if the predicate returns
 * true for all invocations.
 */
uint8_t allList(List* list, uint8_t(*predicate)(void*));

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
 * Returns the length of the list.
 */
uint32_t lengthList(List* list);

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

/**
 * Internal data structure for maps
 */
typedef struct Entry_ {
    struct Entry_* tail;
    void* key;
    void* value;
} Entry;

/**
 * The map type. Unlike lists, operations on maps mutate the map.
 */
typedef struct {
    Entry* entry;
} Map;

Map* createMap();

/**
 * Creates a shallow copy of the map. The structure is copied, but the keys and
 * values are not.
 */
Map* copyMap(Map* map);

/**
 * Destroys the map.
 *
 * @param map the map instance to free
 * @param destroyKey called for each key in the map
 * @param destroyValue called for each value in the map
 */
void destroyMap(Map* map, void(*destroyKey)(void*), void(*destroyValue)(void*));

void* getMapUint32(Map* map, uint32_t key);
void putMapUint32(Map* map, uint32_t key, void* value);

void* getMapStr(Map* map, const char* key);
//holds the reference to key passed to putMap
void putMapStr(Map* map, const char* key, void* value);

/**
 * Allocates memory that holds the given integer value. Useful for storing an
 * integer where a pointer is expected.
 */
uint32_t* boxUint32(uint32_t primitive);

/**
 * Does nothing. Useful for when a callback is expected, but the caller does
 * not need one.
 */
void nothing(void* x);

#endif /* UTIL_H_ */
