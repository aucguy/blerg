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

Thing::~Thing() {}

ThingType TYPE_NONE = 0;
ThingType TYPE_INT = 1;
ThingType TYPE_FLOAT = 2;
ThingType TYPE_STR = 3;
ThingType TYPE_BOOL = 4;
ThingType TYPE_MODULE = 5;
ThingType TYPE_FUNC = 6;
ThingType TYPE_NATIVE_FUNC = 7;
ThingType TYPE_ERROR = 8;
ThingType TYPE_TUPLE = 9;
ThingType TYPE_LIST = 10;
ThingType TYPE_OBJECT = 11;
ThingType TYPE_CELL = 12;
ThingType TYPE_SYMBOL = 13;
ThingType TYPE_VARARG = 14;
ThingType TYPE_UNDEF = 15;
