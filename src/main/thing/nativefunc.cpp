#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/nativefunc.h"

NativeFuncThing::NativeFuncThing(ExecFunc func)
    : func(func) {}
NativeFuncThing::~NativeFuncThing() {}

RetVal NativeFuncThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return this->func(runtime, self, args, arity);
}

RetVal NativeFuncThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return symbolDispatch(runtime, self, args, arity);
}

ThingType NativeFuncThing::type() {
    return TYPE_NATIVE_FUNC;
}

Thing* createNativeFuncThing(Runtime* runtime, ExecFunc func) {
    return createThing(runtime, new NativeFuncThing(func));
}
