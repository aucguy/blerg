#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main/thing.h"
#include "main/execute.h"

RetVal libPrint(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(arity != 1) {
        const char* format = "expected 1 argument but got %i";
        return throwMsg(runtime, formatStr(format , arity));
    }
    if(typeOfThing(args[0]) != THING_TYPE_STR) {
        //TODO report what type the argument actually is
        return throwMsg(runtime, newStr("expected argument 1 to be a string"));
    }

    printf("%s\n", thingAsStr(args[0]));
    return createRetVal(runtime->noneThing, 0);
}

//TODO accept arbitrary long line lengths and remove excess data
RetVal libInput(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    char* str = malloc(sizeof(char) * 100);
    fgets(str, 100, stdin);
    //remove newline
    str[strlen(str) - 1] = 0;
    return createRetVal(createStrThing(runtime, str, 0), 0);
}

RetVal libAssert(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(arity != 1) {
        return throwMsg(runtime, formatStr("expected 1 argument but got %i", arity));
    }

    if(typeOfThing(args[0]) != THING_TYPE_BOOL) {
        //TODO report the actual type
        return throwMsg(runtime, newStr("expected argument 1 to be a bool"));
    }

    if(thingAsBool(args[0]) == 0) {
        return throwMsg(runtime, newStr("assertion failure: boolean is false"));
    }

    return createRetVal(runtime->noneThing, 0);
}

RetVal libToStr(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(arity != 1) {
        return throwMsg(runtime, formatStr("expected 1 argument but got %i", arity));
    }

    if(typeOfThing(args[0]) != THING_TYPE_INT) {
        //TODO report the actual type
        return throwMsg(runtime, newStr("expected argument 1 to be an int"));
    }

    //TODO don't make fixed size
    char* str = malloc(sizeof(char) * 100);
    sprintf(str, "%i", thingAsInt(args[0]));
    return createRetVal(createStrThing(runtime, str, 0), 0);
}

RetVal libToInt(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(arity != 1) {
        return throwMsg(runtime, formatStr("expected 1 argument but got %i", arity));
    }

    if(typeOfThing(args[0]) != THING_TYPE_STR) {
        //TODO report the actual type
        return throwMsg(runtime, newStr("expected argument 1 to be a str"));
    }

    int32_t i = strtol(thingAsStr(args[0]), NULL, 10);
    return createRetVal(createIntThing(runtime, i), 0);
}
