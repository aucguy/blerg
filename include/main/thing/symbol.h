#ifndef STD_LIB_SYMBOL_H_
#define STD_LIB_SYMBOL_H_

#include "main/runtime.h"

class SymbolThing : public Thing {
public:
    uint32_t id;
    uint8_t arity;

    SymbolThing(uint32_t id, uint8_t arity);
    ~SymbolThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* STD_LIB_SYMBOL_H_ */
