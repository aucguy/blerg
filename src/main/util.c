#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#include "main/util.h"

char* newStr(const char* src) {
    uint32_t len = sizeof(char) * (strlen(src) + 1);
    char* dst = (char*) malloc(len);
    memcpy(dst, src, len);
    dst[len - 1] = 0;
    return dst;
}

const char* formatStr(const char* format, ...) {
    va_list args;
    va_start(args, format);

    size_t length = 1;

    for(uint32_t i = 0; format[i] != 0; i++) {
        char c = format[i];

        if(c == '%') {
            i++;
            c = format[i];
            if(c == 's') {
                length += strlen(va_arg(args, char*));
            } else if(c == 'i') {
                int val = va_arg(args, int);
                if(val == 0) {
                    length += 1;
                } else {
                    length += ceil(log10(val));
                }
            } else {
                length++;
            }
        } else if(c == '\\') {
            i++;
            length++;
        } else {
            length++;
        }
    }

    va_end(args);

    char* str = malloc(sizeof(char) * length);
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);

    return str;
}

/**
 * Copies part of a string into a new string.
 *
 * @param str the string to take a slice of
 * @param start the index of the start of the slice
 * @param end the index of the end of the slice
 * @return a string containing the characters from the indexes of 'start' to
 *      'end' in 'src'
 */
char* sliceStr(const char* str, uint32_t start, uint32_t end) {
    uint32_t len = sizeof(char) * (end - start);
    char* extracted = (char*) malloc(len + 1);
    memcpy(extracted, &str[start], len);
    extracted[len] = 0;
    return extracted;
}

List* consList(void* head, List* tail) {
    List* full = (List*) malloc(sizeof(List));
    full->head = head;
    full->tail = tail;
    return full;
}

uint8_t allList2(List* listA, List* listB, uint8_t(*predicate)(void*, void*)) {
    while(listA != NULL && listB != NULL) {
        if(!predicate(listA->head, listB->head)) {
            return 0;
        }
        listA = listA->tail;
        listB = listB->tail;
    }

    return listA == NULL && listB == NULL;
}

uint8_t allList(List* list, uint8_t(*predicate)(void*)) {
    while(list != NULL) {
        if(!predicate(list->head)) {
            return 0;
        }
        list = list->tail;
    }
    return 1;
}

List* mapList(List* list, void*(*func)(void*)) {
    if(list == NULL) {
        return NULL;
    } else {
        return consList(func(list->head), mapList(list->tail, func));
    }
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

uint32_t lengthList(List* list) {
    uint32_t length = 0;
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

Entry* copyEntry(Entry* original) {
    if(original == NULL) {
        return NULL;
    }
    Entry* copy = (Entry*) malloc(sizeof(Entry));
    copy->key = original->key;
    copy->value = original->value;
    copy->tail = copyEntry(original->tail);
    return copy;
}

Map* copyMap(Map* original) {
    Map* copy = (Map*) malloc(sizeof(Map));
    copy->entry = copyEntry(original->entry);
    return copy;
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
uint8_t intEq(const void* a, const void* b) {
    return *((uint8_t*) a) == *((uint8_t*) b);
}

/**
 * Compares two string values
 */
uint8_t strEq(const void* a, const void* b) {
    return strcmp((const char*) a, (const char*) b) == 0;
}

/**
 * Gets the entry object from the map with the given key or NULL if there is
 * no such entry.
 */
Entry* getEntryMap(Map* map, const void* key, uint8_t(*equal)(const void*, const void*)) {
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

void* getMapUint32(Map* map, uint32_t key) {
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

void putMapUint32(Map* map, uint32_t key, void* value) {
    postPutMap(map, getEntryMap(map, &key, intEq), boxUint32(key), value);
}

void putMapStr(Map* map, const char* key, void* value) {
    postPutMap(map, getEntryMap(map, key, strEq), (void*) key, value);
}

uint32_t* boxUint32(uint32_t primitive) {
    uint32_t* boxed = (uint32_t*) malloc(sizeof(uint32_t));
    *boxed = primitive;
    return boxed;
}

