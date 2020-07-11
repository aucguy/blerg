#ifndef THING_MODULE_H_
#define THING_MODULE_H_

#include "main/runtime.h"
#include "main/util.h"

class ModuleThing : public Thing {
public:
    Map* properties;

    ModuleThing(Map* properties);
    ~ModuleThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_MODULE_H_ */
