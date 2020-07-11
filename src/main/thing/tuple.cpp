#include <stdint.h>
#include <stdlib.h>

#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/tuple.h"

TupleThing::TupleThing(uint8_t size, Thing** elements) :
    size(size), elements(elements) {}

TupleThing::~TupleThing() {
    free(this->elements);
}

RetVal TupleThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal TupleThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    //TODO check self is really a symbol first
    uint32_t id = getSymbolId(self);

    if(id == SYM_RESPONDS_TO) {
        if(arity != 2) {
            return throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
        }

        if(typeOfThing(args[1]) != TYPE_SYMBOL) {
            //TODO report the actual type
            const char* msg = "expected argument 2 to be a symbol";
            return throwMsg(runtime, newStr(msg));
        }

        uint32_t checking = getSymbolId(args[1]);
        uint8_t respondsTo =
                checking == SYM_RESPONDS_TO ||
                checking == SYM_EQ ||
                checking == SYM_NOT_EQ ||
                checking == SYM_GET;

        return createRetVal(createBoolThing(runtime, respondsTo), 0);
    } else if(id == SYM_EQ || id == SYM_NOT_EQ) {
        RetVal ret = typeCheck(runtime, self, args, arity, 2, TYPE_TUPLE,
                TYPE_TUPLE);

        if(isRetValError(ret)) {
            return ret;
        }
        uint8_t all = getSymbolId(self) == SYM_EQ;

        TupleThing* valueA = (TupleThing*) args[0];
        TupleThing* valueB = (TupleThing*) args[1];

        if(valueA->size != valueB->size) {
            return createRetVal(createBoolThing(runtime, 0), 0);
        }

        Thing* eqSym = (Thing*) getMapStr(runtime->operators, "==");

        Thing** elems = (Thing**) malloc(sizeof(Thing*) * 2);

        for(uint8_t i = 0; i < valueA->size; i++) {
            elems[0] = valueA->elements[i];
            elems[1] = valueB->elements[i];

            ret = callFunction(runtime, eqSym, 2, elems);
            if(isRetValError(ret)) {
                free(elems);
                return ret;
            }

            Thing* value = getRetVal(ret);

            if(typeOfThing(value) != TYPE_BOOL) {
                free(elems);
                const char* msg = newStr("internal error: == did not return a bool");
                return throwMsg(runtime, msg);
            }

            if(thingAsBool(value) != all) {
                free(elems);
                return ret;
            }
        }

        free(elems);
        return createRetVal(createBoolThing(runtime, all), 0);
    } else if(id == SYM_GET) {
        RetVal ret = typeCheck(runtime, self, args, arity, 2,
                TYPE_TUPLE, TYPE_INT);

        if(isRetValError(ret)) {
            return ret;
        }

        TupleThing* tuple = (TupleThing*) args[0];
        int32_t index = thingAsInt(args[1]);

        if(index >= tuple->size) {
            const char* format = "tuple access out of bounds: "
                    "accessed at %i but the size is %i";
            return throwMsg(runtime, formatStr(format, index, tuple->size));
        }

        return createRetVal(tuple->elements[index], 0);
    } else {
        return throwMsg(runtime, newStr("tuples do not respond to that symbol"));
    }
}

ThingType TupleThing::type() {
    return TYPE_TUPLE;
}

Thing* createTupleThing(Runtime* runtime, uint8_t size, Thing** elements) {
    return createThing(runtime, new TupleThing(size, elements));
}

uint8_t getTupleSize(Thing* tuple) {
    return ((TupleThing*) tuple)->size;
}

Thing* getTupleElem(Thing* tuple, uint8_t index) {
    return ((TupleThing*) tuple)->elements[index];
}
