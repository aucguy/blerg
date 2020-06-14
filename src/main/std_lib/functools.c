#include <stdlib.h>

#include "main/std_lib/functools.h"

#define UNUSED(x) (void)(x)

uint8_t initialized = 0;

RetVal libCall(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    if(arity != 2) {
        return throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
    }

    ThingType* type = typeOfThing(args[1]);
    if(type != THING_TYPE_LIST && type != THING_TYPE_NONE) {
        //TODO report the actual type
        return throwMsg(runtime, formatStr("expected argument 2 to be a list"));
    }

    //TODO extract to its own function once length for lists has been written
    uint8_t count = 0;
    ListThing* list = args[1];
    while(typeOfThing(list) != THING_TYPE_NONE) {
        count++;
        list = list->tail;
    }

    if(count == 0) {
        return throwMsg(runtime, newStr("cannot call function with no arguments"));
    }

    Thing** passedArgs = malloc(sizeof(Thing*) * count);
    list = args[1];
    count = 0;
    while(typeOfThing(list) != THING_TYPE_NONE) {
        passedArgs[count] = list->head;
        count++;
        list = list->tail;
    }

    RetVal ret = callFunction(runtime, args[0], count, passedArgs);
    free(passedArgs);
    return ret;
}

ThingType* THING_TYPE_VARARG = NULL;

typedef struct {
    Thing* func;
} VarargThing;

RetVal varargCall(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(arity < 1) {
        const char* fmt = "expected at least 1 args but got %i";
        return throwMsg(runtime, formatStr(fmt, arity));
    }

    Thing* list = runtime->noneThing;
    for(uint8_t i = arity; i > 0; i--) {
        list = createListThing(runtime, args[i - 1], list);
    }

    Thing* passedArgs[1] = {
            list
    };

    Thing* func = ((VarargThing*) self)->func;

    return callFunction(runtime, func, 1, passedArgs);
}

Thing* createVarargThing(Runtime* runtime, Thing* func) {
    VarargThing* vararg = createThing(runtime, THING_TYPE_VARARG,
            sizeof(VarargThing));
    vararg->func = func;
    return vararg;
}

RetVal libVarargs(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    if(arity != 1) {
        throwMsg(runtime, formatStr("expected 1 arg but got %i", arity));
    }

    return createRetVal(createVarargThing(runtime, args[0]), 0);
}


Thing* initFunctoolsModule(Runtime* runtime) {
    initialized = 1;
    Map* map = createMap();

    THING_TYPE_VARARG = createThingType();
    setDestroyThingType(THING_TYPE_VARARG, destroySimpleThing);
    setCallThingType(THING_TYPE_VARARG, varargCall);
    setDispatchThingType(THING_TYPE_VARARG, symbolDispatch);

    putMapStr(map, "call", createNativeFuncThing(runtime, libCall));
    putMapStr(map, "varargs", createNativeFuncThing(runtime, libVarargs));
    Thing* module = createModuleThing(runtime, map);
    destroyMap(map, nothing, nothing);
    return module;
}

void destroyFunctoolsModule() {
    if(initialized) {
        initialized = 0;

        free(THING_TYPE_VARARG);
        THING_TYPE_VARARG = NULL;
    }
}
