#ifndef THING_TUPLE_H_
#define THING_TUPLE_H_

#include "main/runtime.h"

class TupleThing : public Thing {
public:
    uint8_t size;
    Thing** elements;

    TupleThing(uint8_t size, Thing** elements);
    ~TupleThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_TUPLE_H_ */
