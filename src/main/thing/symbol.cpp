#include <stdint.h>

#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/symbol.h"

SymbolThing::SymbolThing(uint32_t id, uint8_t arity) :
    id(id), arity(arity) {}
SymbolThing::~SymbolThing() {}

RetVal SymbolThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    //TODO remove this check
    if(arity != this->arity && arity != 0) {
        const char* format = "expected %i arguments, but got %i";
        return throwMsg(runtime, formatStr(format, this->arity, arity));
    }
    return args[0]->dispatch(runtime, self, args, arity);
}

RetVal SymbolThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return symbolDispatch(runtime, self, args, arity);
}

ThingType SymbolThing::type() {
    return TYPE_SYMBOL;
}

uint32_t symbolId = 0;

uint32_t newSymbolId() {
    return symbolId++;
}

Thing* createSymbolThing(Runtime* runtime, uint32_t id, uint8_t arity) {
    return createThing(runtime, new SymbolThing(id, arity));
}

uint32_t getSymbolId(Thing* self) {
    return ((SymbolThing*) self)->id;
}
