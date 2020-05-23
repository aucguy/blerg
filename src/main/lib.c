#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main/thing.h"
#include "main/execute.h"

#define UNUSED(x) (void)(x)

RetVal libPrint(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_STR);
    if(isRetValError(retVal)) {
        return retVal;
    }

    printf("%s\n", thingAsStr(args[0]));
    return createRetVal(runtime->noneThing, 0);
}

//TODO accept arbitrary long line lengths and remove excess data
RetVal libInput(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    UNUSED(args);
    UNUSED(arity);

    char* str = malloc(sizeof(char) * 100);
    fgets(str, 100, stdin);
    //remove newline
    str[strlen(str) - 1] = 0;
    return createRetVal(createStrThing(runtime, str, 0), 0);
}

RetVal libAssert(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    UNUSED(arity);

    RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_BOOL);
    if(isRetValError(retVal)) {
        return retVal;
    }

    if(thingAsBool(args[0]) == 0) {
        return throwMsg(runtime, newStr("assertion failure: argument is false"));
    }

    return createRetVal(runtime->noneThing, 0);
}

RetVal libToStr(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_INT);
    if(isRetValError(retVal)) {
        return retVal;
    }

    //TODO don't make fixed size
    char* str = malloc(sizeof(char) * 100);
    sprintf(str, "%i", thingAsInt(args[0]));
    return createRetVal(createStrThing(runtime, str, 0), 0);
}

RetVal libToInt(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_STR);
    if(isRetValError(retVal)) {
        return retVal;
    }
    int32_t i = strtol(thingAsStr(args[0]), NULL, 10);
    return createRetVal(createIntThing(runtime, i), 0);
}

RetVal libTryCatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(arity != 2) {
        return throwMsg(runtime, formatStr("expected 2 arguments but got %i"));
    }

    Thing* block1 = args[0];
    Thing* block2 = args[1];

    Thing** passedArgs = malloc(sizeof(Thing*) * 1);
    passedArgs[0] = runtime->noneThing;

    RetVal result = callFunction(runtime, block1, 1, passedArgs);

    if(!isRetValError(result)) {
        return result;
    }

    passedArgs[0] = getRetVal(result);
    return callFunction(runtime, block2, 1, passedArgs);
}

