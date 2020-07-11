#include "main/thing.h"
#include "main/thing/int.h"

IntThing::IntThing(int32_t value) :
    value(value) {}

IntThing::~IntThing() {}

RetVal IntThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal IntThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    uint32_t id = getSymbolId(self);

    if(id == SYM_RESPONDS_TO) {
        if(arity != 2) {
            throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
        }

        if(typeOfThing(args[1]) != TYPE_SYMBOL) {
            //TODO report the type
            throwMsg(runtime, formatStr("expected argument 2 to be a symbol"));
        }

        uint32_t checking = getSymbolId(args[1]);

        uint32_t respondsTo =
                checking == SYM_RESPONDS_TO ||
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
        RetVal retVal = typeCheck(runtime, self, args, arity, 2, TYPE_INT,
                TYPE_INT);
        if(isRetValError(retVal)) {
            return retVal;
        }

        int32_t valueA = thingAsInt(args[0]);
        int32_t valueB = thingAsInt(args[1]);

        Thing* thing = NULL;
        const char* error = NULL;

        if(id == SYM_ADD) {
            thing = createIntThing(runtime, valueA + valueB);
        } else if(id == SYM_SUB) {
            thing = createIntThing(runtime, valueA - valueB);
        } else if(id == SYM_MUL) {
            thing = createIntThing(runtime, valueA * valueB);
        } else if(id == SYM_DIV) {
            thing = createIntThing(runtime, valueA / valueB);
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
            //this case shouldn't happen
            error = "ints do not respond to that symbol";
        }

        if(error != NULL) {
            return throwMsg(runtime, error);
        } else {
            return createRetVal(thing, 0);
        }
    }
}

ThingType IntThing::type() {
    return TYPE_INT;
}

/**
 * Represents integers found in the source code.
 */
Thing* createIntThing(Runtime* runtime, int32_t value) {
    return createThing(runtime, new IntThing(value));
}

int32_t thingAsInt(Thing* thing) {
    return ((IntThing*) thing)->value;
}
