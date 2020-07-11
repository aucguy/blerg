#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "main/execute.h"
#include "main/runtime.h"
#include "main/thing.h"

#include "main/thing/none.h"
#include "main/thing/int.h"
#include "main/thing/float.h"
#include "main/thing/symbol.h"
#include "main/thing/str.h"
#include "main/thing/bool.h"
#include "main/thing/module.h"
#include "main/thing/func.h"
#include "main/thing/nativefunc.h"
#include "main/thing/tuple.h"
#include "main/thing/error.h"
#include "main/thing/list.h"
#include "main/thing/object.h"
#include "main/thing/cell.h"

RetVal callFail(Runtime* runtime) {
    return throwMsg(runtime, "cannot call this type");
}

//TODO rename function
RetVal symbolDispatch(Runtime* runtime, Thing* self, Thing** args,
        uint8_t arity) {
    if(getSymbolId(self) == SYM_RESPONDS_TO) {
        if(arity != 2) {
            const char* fmt = "expected 2 args but got %i";
            return throwMsg(runtime, formatStr(fmt, arity));
        }

        if(typeOfThing(args[1]) != TYPE_SYMBOL) {
            //TODO report the actual type
            const char* msg = "expected argument 2 to be a symbol";
            return throwMsg(runtime, newStr(msg));
        }

        uint8_t respondsTo = getSymbolId(args[1]) == SYM_RESPONDS_TO;

        return createRetVal(createBoolThing(runtime, respondsTo), 0);
    } else {
        //TODO report the actual symbol
        return throwMsg(runtime, newStr("symbols to not respond to that symbol"));
    }
}

uint32_t SYM_ADD = 0;
uint32_t SYM_SUB = 0;
uint32_t SYM_MUL = 0;
uint32_t SYM_DIV = 0;
uint32_t SYM_EQ = 0;
uint32_t SYM_NOT_EQ = 0;
uint32_t SYM_LESS_THAN = 0;
uint32_t SYM_LESS_THAN_EQ = 0;
uint32_t SYM_GREATER_THAN = 0;
uint32_t SYM_GREATER_THAN_EQ = 0;
uint32_t SYM_AND = 0;
uint32_t SYM_OR = 0;
uint32_t SYM_NOT = 0;
uint32_t SYM_GET = 0;
uint32_t SYM_DOT = 0;
uint32_t SYM_CALL = 0;
uint32_t SYM_RESPONDS_TO = 0;
uint32_t SYM_UNPACK = 0;

#define UNUSED(x) (void)(x)

void destroySimpleThing(Thing* thing) {
    UNUSED(thing);
}

RetVal errorCall(Runtime* runtime, Thing* thing, Thing** args, uint8_t arity) {
    UNUSED(runtime);
    UNUSED(thing);
    UNUSED(arity);
    UNUSED(args);
    //TODO report the object type
    return throwMsg(runtime, "cannot call this type");
}

static uint8_t initialized = 0;

void initThing() {
    if(!initialized) {
        SYM_ADD = newSymbolId();
        SYM_SUB = newSymbolId();
        SYM_MUL = newSymbolId();
        SYM_DIV = newSymbolId();
        SYM_EQ = newSymbolId();
        SYM_NOT_EQ = newSymbolId();
        SYM_LESS_THAN = newSymbolId();
        SYM_LESS_THAN_EQ = newSymbolId();
        SYM_GREATER_THAN = newSymbolId();
        SYM_GREATER_THAN_EQ = newSymbolId();
        SYM_AND = newSymbolId();
        SYM_OR = newSymbolId();
        SYM_NOT = newSymbolId();
        SYM_GET = newSymbolId();
        SYM_DOT = newSymbolId();
        SYM_CALL = newSymbolId();
        SYM_RESPONDS_TO = newSymbolId();
        SYM_UNPACK = newSymbolId();

        initialized = 1;
    }
}

void deinitThing() {
    if(initialized) {
        SYM_ADD = 0;
        SYM_SUB = 0;
        SYM_MUL = 0;
        SYM_DIV = 0;
        SYM_EQ = 0;
        SYM_LESS_THAN = 0;
        SYM_LESS_THAN_EQ = 0;
        SYM_GREATER_THAN = 0;
        SYM_GREATER_THAN_EQ = 0;
        SYM_AND = 0;
        SYM_OR = 0;
        SYM_NOT = 0;
        SYM_GET = 0;
        SYM_DOT = 0;
        SYM_CALL = 0;

        initialized = 0;
    }
}

/**
 * Creates a thing. If the runtime is destroyed, the object will also be
 * destroyed.
 *
 * @param runtime the runtime object
 * @param type the native type of the object
 * @param size the size of the object's custom data. The custom data is found
 *      after the Thing struct.
 */
Thing* createThing(Runtime* runtime, Thing* type) {
    runtime->allocatedThings = consList(type, runtime->allocatedThings);
    return type;
}

void destroyThing(Thing* thing) {
    delete thing;
}

ThingType typeOfThing(Thing* thing) {
    return thing->type();
}

RetVal typeCheck(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t expectedArity, ...) {
    UNUSED(self);

    if(arity != expectedArity) {
        const char* msg = formatStr("expected %i arguments but got %i",
                expectedArity, arity);
        return throwMsg(runtime, msg);
    }

    va_list types;
    va_start(types, expectedArity);


    for(uint8_t i = 0; i < arity; i++) {
        if(typeOfThing(args[i]) != va_arg(types, ThingType)) {
            va_end(types);
            return throwMsg(runtime, formatStr("wrong type for argument %i", i + 1));
        }
    }

    va_end(types);

    return createRetVal(NULL, 0);
}
