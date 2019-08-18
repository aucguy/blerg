#ifndef UTIL_H_
#define UTIL_H_

const char* newStr(const char* src);

typedef struct List_ {
    void* head;
    struct List_* tail;
} List;

List* consList(void* head, List* tail);
void destroyList(List* list, void(*func)(void*));

#endif /* UTIL_H_ */
