#include <string.h>

#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/str.h"

StrThing::StrThing(const char* value, uint8_t literal) :
    value(value), literal(literal) {}

StrThing::~StrThing() {
    if(!this->literal) {
        free((char*) this->value);
    }
}

RetVal StrThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal StrThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
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
                checking == SYM_EQ ||
                checking == SYM_NOT_EQ;

        return createRetVal(createBoolThing(runtime, respondsTo), 0);
    } else {
        RetVal retVal = typeCheck(runtime, self, args, arity, 2, TYPE_STR,
                TYPE_STR);
        if(isRetValError(retVal)) {
            return retVal;
        }

        const char* valueA = thingAsStr(args[0]);
        const char* valueB = thingAsStr(args[1]);

        Thing* thing;
        const char* error = 0;

        if(id == SYM_ADD) {
            uint32_t len = strlen(valueA) + strlen(valueB);
            char* out = (char*) malloc(sizeof(char) * (len + 1));
            out[0] = 0;
            strcat(out, valueA);
            strcat(out, valueB);
            thing = createStrThing(runtime, out, 0);
        } else if(id == SYM_EQ) {
            thing = createBoolThing(runtime, strcmp(valueA, valueB) == 0);
        } else if(id == SYM_NOT_EQ) {
            thing = createBoolThing(runtime, strcmp(valueA, valueB) != 0);
        } else {
            //TODO report the symbol
            error = "strs do not respond to that symbol";
        }

        if(error != NULL) {
            return throwMsg(runtime, error);
        } else {
            return createRetVal(thing, 0);
        }
    }
}

ThingType StrThing::type() {
    return TYPE_STR;
}

Thing* createStrThing(Runtime* runtime, const char* value, uint8_t literal) {
    return createThing(runtime, new StrThing(value, literal));
}

const char* thingAsStr(Thing* self) {
    return ((StrThing*) self)->value;
}
