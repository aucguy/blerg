#ifndef THING_ERROR_H_
#define THING_ERROR_H_

#include "main/runtime.h"

typedef struct {
    uint8_t native;
    SrcLoc location;
    const char* filename;
} ErrorFrame;

class ErrorThing : public Thing {
public:
    const char* msg;
    List* stackFrame;

    ErrorThing(const char* msg, List* stackFrame);
    ~ErrorThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_ERROR_H_ */
