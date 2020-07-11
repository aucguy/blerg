#ifndef THING_BOOL_H_
#define THING_BOOL_H_

#include "main/runtime.h"

class BoolThing : public Thing {
public:
    uint8_t value;

    BoolThing(uint8_t value);
    ~BoolThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_BOOL_H_ */
