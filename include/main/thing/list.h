#ifndef THING_LIST_H_
#define THING_LIST_H_

#include "main/runtime.h"

class ListThing : public Thing {
public:
    Thing* head;
    Thing* tail;

    ListThing(Thing* head, Thing* tail);
    ~ListThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_LIST_H_ */
