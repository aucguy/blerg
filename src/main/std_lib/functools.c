#include <stdlib.h>

#include "main/std_lib/functools.h"

#define UNUSED(x) (void)(x)

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

RetVal libVarargs(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    if(arity < 2) {
        const char* fmt = "expected at least 1 args but got %i";
        return throwMsg(runtime, formatStr(fmt, arity));
    }

    Thing* func = args[0];

    Thing* list = runtime->noneThing;
    for(uint8_t i = arity - 1; i > 0; i--) {
        list = createListThing(runtime, args[i], list);
    }

    Thing* passedArgs[1] = {
            list
    };

    return callFunction(runtime, func, 1, passedArgs);
}


Thing* initFunctoolsModule(Runtime* runtime) {
    Map* map = createMap();
    putMapStr(map, "call", createNativeFuncThing(runtime, libCall));
    putMapStr(map, "varargs", createNativeFuncThing(runtime, libVarargs));
    Thing* module = createModuleThing(runtime, map);
    destroyMap(map, nothing, nothing);
    return module;
}
