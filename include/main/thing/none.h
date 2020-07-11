#ifndef THING_NONE_H_
#define THING_NONE_H_

#include "main/runtime.h"
#include "main/thing.h"

class NoneThing : public Thing {
public:
    NoneThing();
    ~NoneThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};


#endif /* THING_NONE_H_ */
