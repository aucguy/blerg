#ifndef THING_NATIVEFUNC_H_
#define THING_NATIVEFUNC_H_

#include "main/runtime.h"

class NativeFuncThing : public Thing {
public:
    ExecFunc func;

    NativeFuncThing(ExecFunc func);
    ~NativeFuncThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_NATIVEFUNC_H_ */
