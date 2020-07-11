#ifndef THING_FLOAT_H_
#define THING_FLOAT_H_

#include "main/runtime.h"

class FloatThing : public Thing {
public:
    float value;

    FloatThing(float value);
    ~FloatThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) ;
    ThingType type();
};


#endif /* THING_FLOAT_H_ */
