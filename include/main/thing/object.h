#ifndef THING_OBJECT_H_
#define THING_OBJECT_H_

#include "main/runtime.h"

class ObjectThing : public Thing {
public:
    Map* map;

    ObjectThing(Map* map);
    ~ObjectThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_OBJECT_H_ */
