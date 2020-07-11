#ifndef THING_FUNC_H_
#define THING_FUNC_H_

#include "main/runtime.h"

class FuncThing : public Thing {
public:
    //location of first bytecode of the function
    unsigned int entry;
    //module the function was declared in
    Module* module;
    //the scope the function was declared in
    Scope* parentScope;

    FuncThing(unsigned int entry, Module* module, Scope* parentScope);
    ~FuncThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_FUNC_H_ */
