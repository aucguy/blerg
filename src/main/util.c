#include <stdlib.h>
#include <string.h>

#include "main/util.h"

char* newStr(const char* src) {
    int len = sizeof(char) * (strlen(src) + 1);
    char* dst = (char*) malloc(len);
    memcpy(dst, src, len);
    dst[len - 1] = 0;
    return dst;
}

List* consList(void* head, List* tail) {
    List* full = (List*) malloc(sizeof(List));
    full->head = head;
    full->tail = tail;
    return full;
}

int allList2(List* listA, List* listB, int(*predicate)(void*, void*)) {
    while(listA != NULL && listB != NULL) {
        if(!predicate(listA->head, listB->head)) {
            return 0;
        }
        listA = listA->tail;
        listB = listB->tail;
    }

    return listA == NULL && listB == NULL;
}

int allList(List* list, int(*predicate)(void*)) {
    while(list != NULL) {
        if(!predicate(list->head)) {
            return 0;
        }
        list = list->tail;
    }
    return 1;
}

void destroyList(List* list, void(*func)(void*)) {
    if(list == NULL) {
        return;
    }
    destroyList(list->tail, func);
    func(list->head);
    free(list);
}

void nothing(void* x) {
    (void)(x);
}

void destroyShallowList(List* list) {
    destroyList(list, nothing);
}

void* lastList(List* list) {
    while(list->tail != NULL) {
        list = list->tail;
    }
    return list->head;
}

int lengthList(List* list) {
    int length = 0;
    while(list != NULL) {
        list = list->tail;
        length++;
    }
    return length;
}

List* prependReverseList(List* before, List* after) {
    while(before != NULL) {
        after = consList(before->head, after);
        before = before->tail;
    }
    return after;
}

List* reverseList(List* list) {
    List* reversed = NULL;
    while(list != NULL) {
        reversed = consList(list->head, reversed);
        list = list->tail;
    }
    return reversed;
}

Map* createMap() {
    Map* map = (Map*) malloc(sizeof(Map));
    map->entry = NULL;
    return map;
}

void destroyEntry(Entry* entry, void(*destroyKey)(void*), void(*destroyValue)(void*)) {
    if(entry == NULL) {
        return;
    }
    destroyKey(entry->key);
    destroyValue(entry->value);
    destroyEntry(entry->tail, destroyKey, destroyValue);
    free(entry);
}

void destroyMap(Map* map, void(*destroyKey)(void*), void(*destroyValue)(void*)) {
    destroyEntry(map->entry, destroyKey, destroyValue);
    free(map);
}

/**
 * Compares two boxed integer values
 */
int intEq(const void* a, const void* b) {
    return *((int*) a) == *((int*) b);
}

/**
 * Compares two string values
 */
int strEq(const void* a, const void* b) {
    return strcmp((const char*) a, (const char*) b) == 0;
}

/**
 * Gets the entry object from the map with the given key or NULL if there is
 * no such entry.
 */
Entry* getEntryMap(Map* map, const void* key, int(*equal)(const void*, const void*)) {
    for(Entry* i = map->entry; i != NULL; i = i->tail) {
        if(equal(i->key, key)) {
            return i;
        }
    }
    return NULL;
}

void* postGetMap(Entry* entry) {
    if(entry == NULL) {
        return NULL;
    } else {
        return entry->value;
    }
}

void* getMapInt(Map* map, int key) {
    return postGetMap(getEntryMap(map, &key, intEq));
}

void* getMapStr(Map* map, const char* key) {
    return postGetMap(getEntryMap(map, key, strEq));
}

void postPutMap(Map* map, Entry* entry, void* key, void* value) {
    if(entry == NULL) {
        entry = (Entry*) malloc(sizeof(Entry));
        entry->tail = map->entry;
        entry->key = key;
        entry->value = value;
        map->entry = entry;
    } else {
        entry->value = value;
    }
}

void putMapInt(Map* map, int key, void* value) {
    postPutMap(map, getEntryMap(map, &key, intEq), boxInt(key), value);
}

void putMapStr(Map* map, const char* key, void* value) {
    postPutMap(map, getEntryMap(map, key, strEq), (void*) key, value);
}

int* boxInt(int primitive) {
    int* boxed = (int*) malloc(sizeof(int));
    *boxed = primitive;
    return boxed;
}

