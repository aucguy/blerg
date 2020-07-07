#include "main/runtime.h"

RetVal createRetVal(Thing* value, uint8_t error) {
    RetVal retVal;
    retVal.value = value;
    retVal.error = error;
    return retVal;
}

uint8_t isRetValError(RetVal val) {
    return val.error;
}
Thing* getRetVal(RetVal val) {
    return val.value;
}

RetVal throwMsg(Runtime* runtime, const char* msg) {
    return createRetVal(createErrorThing(runtime, msg), 1);
}

ThingType::~ThingType() {
}

LegacyThingType::LegacyThingType()
    : destroyFunc(nullptr), callFunc(nullptr), dispatchFunc(nullptr) {
}

LegacyThingType::~LegacyThingType() {
}

void LegacyThingType::destroy(Thing* thing) {
    this->destroyFunc(thing);
}

RetVal LegacyThingType::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return this->callFunc(runtime, self, args, arity);
}

RetVal LegacyThingType::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return this->dispatchFunc(runtime, self, args, arity);
}

