#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/float.h"

FloatThing::FloatThing(float value) :
        value(value) {}

FloatThing::~FloatThing() {}

RetVal FloatThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal FloatThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    uint32_t id = getSymbolId(self);
    if(id == SYM_RESPONDS_TO) {
        if(arity != 2) {
            const char* fmt = "expected 2 args but got %i";
            return throwMsg(runtime, formatStr(fmt, arity));
        }

        if(typeOfThing(args[1]) != TYPE_SYMBOL) {
            //TODO report the actual type
            const char* msg = "expected argument 2 to be a symbol";
            return throwMsg(runtime, newStr(msg));
        }

        uint32_t checking = getSymbolId(args[1]);
        uint8_t respondsTo =
                checking == SYM_ADD ||
                checking == SYM_SUB ||
                checking == SYM_MUL ||
                checking == SYM_DIV ||
                checking == SYM_EQ ||
                checking == SYM_NOT_EQ ||
                checking == SYM_LESS_THAN ||
                checking == SYM_LESS_THAN_EQ ||
                checking == SYM_GREATER_THAN ||
                checking == SYM_GREATER_THAN_EQ;

        return createRetVal(createBoolThing(runtime, respondsTo), 0);
    } else {
        RetVal retVal = typeCheck(runtime, self, args, arity, 2,
                TYPE_FLOAT, TYPE_FLOAT);
        if(isRetValError(retVal)) {
            return retVal;
        }

        float valueA = ((FloatThing*) args[0])->value;
        float valueB = ((FloatThing*) args[1])->value;

        Thing* thing = NULL;
        const char* error = NULL;

        if(id == SYM_ADD) {
            thing = createFloatThing(runtime, valueA + valueB);
        } else if(id == SYM_SUB) {
            thing = createFloatThing(runtime, valueA - valueB);
        } else if(id == SYM_MUL) {
            thing = createFloatThing(runtime, valueA * valueB);
        } else if(id == SYM_DIV) {
            thing = createFloatThing(runtime, valueA / valueB);
        } else if(id == SYM_EQ) {
            thing = createBoolThing(runtime, (uint8_t) (valueA == valueB));
        } else if(id == SYM_NOT_EQ) {
            thing = createBoolThing(runtime, (uint8_t) (valueA != valueB));
        } else if(id == SYM_LESS_THAN) {
            thing = createBoolThing(runtime, (uint8_t) (valueA < valueB));
        } else if(id == SYM_LESS_THAN_EQ) {
            thing = createBoolThing(runtime, (uint8_t) (valueA <= valueB));
        } else if(id == SYM_GREATER_THAN) {
            thing = createBoolThing(runtime, (uint8_t) (valueA > valueB));
        } else if(id == SYM_GREATER_THAN_EQ) {
            thing = createBoolThing(runtime, (uint8_t) (valueA >= valueB));
        } else {
            //TODO report the symbol
            error = "floats do not respond to that symbol";
        }

        if(error != NULL) {
            return throwMsg(runtime, error);
        } else {
            return createRetVal(thing, 0);
        }
    }
}

ThingType FloatThing::type() {
    return TYPE_FLOAT;
}

Thing* createFloatThing(Runtime* runtime, float value) {
    return createThing(runtime, new FloatThing(value));
}

float thingAsFloat(Thing* thing) {
    return ((FloatThing*) thing)->value;
}
