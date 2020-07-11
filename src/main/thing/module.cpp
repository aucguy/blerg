#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/module.h"

ModuleThing::ModuleThing(Map* properties) :
    properties(properties) {}

ModuleThing::~ModuleThing() {
    destroyMap(this->properties, nothing, nothing);
}

RetVal ModuleThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal ModuleThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(getSymbolId(self) == SYM_RESPONDS_TO) {
        if(arity != 2) {
            return throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
        }

        if(typeOfThing(args[1]) != TYPE_SYMBOL) {
            //TODO report the actual type
            const char* msg = "expected argument 2 to be a symbol";
            return throwMsg(runtime, newStr(msg));
        }

        uint8_t respondsTo = getSymbolId(args[1]) == SYM_DOT;

        return createRetVal(createBoolThing(runtime, respondsTo), 0);
    } else {
        RetVal ret = typeCheck(runtime, self, args, arity, 2, TYPE_MODULE,
                TYPE_STR);
        if(isRetValError(ret)) {
            return ret;
        }

        if(getSymbolId(self) != SYM_DOT) {
            //TODO report the actual symbol
            const char* msg = newStr("modules do not respond to that symbol");
            return throwMsg(runtime, msg);
        }

        const char* name = thingAsStr(args[1]);
        Thing* property = getModuleProperty(args[0], name);
        if(property == NULL) {
            return throwMsg(runtime, formatStr("export '%s' not found", name));
        }

        return createRetVal(property, 0);
    }
}

ThingType ModuleThing::type() {
    return TYPE_MODULE;
}

/**
 * Used for module objects and object literals. Associates strings with things.
 */
Thing* createModuleThing(Runtime* runtime, Map* map) {
    return createThing(runtime, new ModuleThing(copyMap(map)));
}

Thing* getModuleProperty(Thing* thing, const char* name) {
    return (Thing*) getMapStr(((ModuleThing*) thing)->properties, name);
}
