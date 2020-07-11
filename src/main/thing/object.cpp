#include "main/runtime.h"
#include "main/execute.h"
#include "main/thing.h"
#include "main/thing/object.h"

ObjectThing::ObjectThing(Map* map) :
    map(map) {}

ObjectThing::~ObjectThing() {
    destroyMap(this->map, free, nothing);
}

RetVal ObjectThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    //these typechecks should pass, but they are here just in case
    if(arity == 0) {
        const char* fmt = "expected at least 1 args but got %i";
        return throwMsg(runtime, formatStr(fmt, arity));
    }

    if(typeOfThing(self) != TYPE_OBJECT) {
        //TODO report the actual type
        return throwMsg(runtime, "expected self to be an object");
    }

    Thing* value = (Thing*) getMapUint32(this->map, SYM_CALL);
    if(value == NULL) {
        const char* msg = "object is not callable (does not have call property)";
        return throwMsg(runtime, newStr(msg));
    } else {
        return callFunction(runtime, value, arity, args);
    }
}

RetVal ObjectThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    //these typechecks should pass, but they for here just in case
    if(arity == 0) {
        const char* fmt = "expected at least 1 args but got %i";
        return throwMsg(runtime, formatStr(fmt, arity));
    }

    if(typeOfThing(args[0]) != TYPE_OBJECT) {
        //TODO report the actual type
        return throwMsg(runtime, "expected argument 1 to be an object");
    }

    Thing* value = (Thing*) getMapUint32(this->map, getSymbolId(self));
    if(value == NULL) {
        if(getSymbolId(self) == SYM_RESPONDS_TO) {
            if(arity != 2) {
                const char* fmt = "expected 2 arg but got %i";
                return throwMsg(runtime, formatStr(fmt, arity));
            }

            if(typeOfThing(args[1]) != TYPE_SYMBOL) {
                //TODO report the actual type
                const char* msg = "expected argument 2 to be a symbol";
                return throwMsg(runtime, newStr(msg));
            }

            uint32_t checking = getSymbolId(args[1]);
            uint32_t responds = getMapUint32(this->map, checking) != NULL;
            return createRetVal(createBoolThing(runtime, responds), 0);
        } else {
            const char* msg = "object does not respond to that symbol";
            return throwMsg(runtime, newStr(msg));
        }
    } else if(arity == 1) {
        return createRetVal(value, 0);
    } else {
        Thing** passedArgs = (Thing**) malloc(sizeof(Thing*) * (arity - 1));
        for(uint8_t i = 1; i < arity; i++) {
            passedArgs[i - 1] = args[i];
        }

        RetVal ret = callFunction(runtime, value, arity - 1, passedArgs);
        free(passedArgs);
        return ret;
    }
}

ThingType ObjectThing::type() {
    return TYPE_OBJECT;
}

Thing* createObjectThing(Runtime* runtime, Map* map) {
    return createThing(runtime, new ObjectThing(map));
}

Map* getObjectMap(Thing* object) {
    return ((ObjectThing*) object)->map;
}
