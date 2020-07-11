#include "main/runtime.h"
#include "main/thing.h"

#include "main/thing/none.h"

NoneThing::NoneThing() {}
NoneThing::~NoneThing() {}

RetVal NoneThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal NoneThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

ThingType NoneThing::type() {
    return TYPE_NONE;
}

/**
 * Singleton none object.
 */
Thing* createNoneThing(Runtime* runtime) {
    return createThing(runtime, new NoneThing());
}
