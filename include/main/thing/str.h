#ifndef THING_STR_H_
#define THING_STR_H_

#include "main/runtime.h"

class StrThing : public Thing {
public:
    const char* value;
    uint8_t literal;

    StrThing(const char* value, uint8_t literal);
    ~StrThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_STR_H_ */
