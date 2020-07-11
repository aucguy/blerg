#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/bool.h"

BoolThing::BoolThing(uint8_t value) :
    value(value) {}

BoolThing::~BoolThing() {}

RetVal BoolThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal BoolThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(typeOfThing(self) != TYPE_SYMBOL) {\
        const char* msg = "internal error: self should be a symbol";
        return throwMsg(runtime, newStr(msg));
    }

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
                checking == SYM_NOT ||
                checking == SYM_ADD ||
                checking == SYM_NOT;

        return createRetVal(createBoolThing(runtime, respondsTo), 0);
    } else {
        Thing* thing;
        const char* error = NULL;

        if(id == SYM_NOT) {
            RetVal retVal = typeCheck(runtime, self, args, arity, 1,
                    TYPE_BOOL);
            if(isRetValError(retVal)) {
                return retVal;
            }

            thing = createBoolThing(runtime, !thingAsBool(args[0]));
        } else {
            RetVal retVal = typeCheck(runtime, self, args, arity, 2,
                    TYPE_BOOL, TYPE_BOOL);
            if(isRetValError(retVal)) {
                return retVal;
            }

            uint8_t valueA = thingAsBool(args[0]);
            uint8_t valueB = thingAsBool(args[1]);

            if(id == SYM_AND) {
                thing = createBoolThing(runtime, valueA && valueB);
            } else if(id == SYM_OR) {
                thing = createBoolThing(runtime, valueA || valueB);
            } else {
                //TODO report the symbol
                error = newStr("bools do not respond to that symbol");
            }
        }

        if(error != NULL) {
            return throwMsg(runtime, error);
        } else {
            return createRetVal(thing, 0);
        }
    }
}

ThingType BoolThing::type() {
    return TYPE_BOOL;
}

Thing* createBoolThing(Runtime* runtime, uint8_t value) {
    return createThing(runtime, new BoolThing(value));
}

uint8_t thingAsBool(Thing* thing) {
    return ((BoolThing*) thing)->value;
}

