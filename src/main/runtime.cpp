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
