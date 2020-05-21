#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main/thing.h"
#include "main/execute.h"

RetVal libPrint(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(arity != 1 || typeOfThing(args[0]) != THING_TYPE_STR) {
        return createRetVal(NULL, 1);
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

Thing* libAssert(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error) {
    if(arity != 1) {
        *error = 1;
        return NULL;
    }

    if(typeOfThing(args[0]) != THING_TYPE_BOOL) {
        *error = 1;
        return NULL;
    }

    if(thingAsBool(args[0]) == 0) {
        *error = 1;
        return NULL;
    }

    return runtime->noneThing;
}

Thing* libToStr(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error) {
    if(arity != 1) {
        *error = 1;
        return NULL;
    }

    if(typeOfThing(args[0]) != THING_TYPE_INT) {
        *error = 1;
        return NULL;
    }

    //TODO don't make fixed size
    char* str = malloc(sizeof(char) * 100);
    sprintf(str, "%i", thingAsInt(args[0]));
    return createStrThing(runtime, str, 0);
}

Thing* libToInt(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error) {
    if(arity != 1) {
        *error = 1;
        return NULL;
    }

    if(typeOfThing(args[0]) != THING_TYPE_STR) {
        *error = 1;
        return NULL;
    }

    int32_t i = strtol(thingAsStr(args[0]), NULL, 10);
    return createIntThing(runtime, i);
}
