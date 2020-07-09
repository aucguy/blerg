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

Thing::~Thing() {
}

ThingTypes TYPE_NONE = 0;
ThingTypes TYPE_INT = 1;
ThingTypes TYPE_FLOAT = 2;
ThingTypes TYPE_STR = 3;
ThingTypes TYPE_BOOL = 4;
ThingTypes TYPE_MODULE = 5;
ThingTypes TYPE_FUNC = 6;
ThingTypes TYPE_NATIVE_FUNC = 7;
ThingTypes TYPE_ERROR = 8;
ThingTypes TYPE_TUPLE = 9;
ThingTypes TYPE_LIST = 10;
ThingTypes TYPE_OBJECT = 11;
ThingTypes TYPE_CELL = 12;
ThingTypes TYPE_SYMBOL = 13;
ThingTypes TYPE_VARARG = 14;
ThingTypes TYPE_UNDEF = 15;
