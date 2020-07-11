#ifndef THING_INT_H_
#define THING_INT_H_

#include <stdint.h>

#include "main/runtime.h"

class IntThing : public Thing {
public:
    int32_t value;

    IntThing(int32_t value);
    ~IntThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_INT_H_ */
