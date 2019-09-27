#include <stdlib.h>
#include <string.h>
#include "main/util.h"

const char* newStr(const char* src) {
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

int allList(List* listA, List* listB, int(*predicate)(void*, void*)) {
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
