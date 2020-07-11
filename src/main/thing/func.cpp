#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/func.h"

FuncThing::FuncThing(unsigned int entry, Module* module, Scope* parentScope) :
    entry(entry), module(module), parentScope(parentScope) {}

FuncThing::~FuncThing() {}

RetVal FuncThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal FuncThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return symbolDispatch(runtime, self, args, arity);
}

ThingType FuncThing::type() {
    return TYPE_FUNC;
}

unsigned int getFuncEntry(Thing* thing) {
    return ((FuncThing*) thing)->entry;
}

Module* getFuncModule(Thing* thing) {
    return ((FuncThing*) thing)->module;
}

Scope* getFuncParentScope(Thing* thing) {
    return ((FuncThing*) thing)->parentScope;
}

Thing* createFuncThing(Runtime* runtime, uint32_t entry,
        Module* module, Scope* parentScope) {
    return createThing(runtime, new FuncThing(entry, module, parentScope));
}
