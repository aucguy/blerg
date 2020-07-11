#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/list.h"

ListThing::ListThing(Thing* head, Thing* tail) :
    head(head), tail(tail) {}
ListThing::~ListThing() {}

RetVal ListThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal ListThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

ThingType ListThing::type() {
    return TYPE_LIST;
}

Thing* getListHead(Thing* thing) {
    return ((ListThing*) thing)->head;
}

Thing* getListTail(Thing* thing) {
    return ((ListThing*) thing)->tail;
}

Thing* createListThing(Runtime* runtime, Thing* head, Thing* tail) {
    return createThing(runtime, new ListThing(head, tail));
}

