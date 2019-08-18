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

void destroyList(List* list, void(*func)(void*)) {
    if(list == NULL) {
        return;
    }
    destroyList(list->tail, func);
    func(list->head);
    free(list);
}
