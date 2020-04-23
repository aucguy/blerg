#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "main/thing.h"
#include "main/execute.h"

Thing* libPrint(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error) {
    if(arity != 1 || typeOfThing(args[0]) != THING_TYPE_STR) {
        *error = 1;
        return NULL;
    }

    printf("%s\n", thingAsStr(args[0]));
    return runtime->noneThing;
}

//TODO accept arbitrary long line lengths and remove excess data
Thing* libInput(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error) {

    char* str = malloc(sizeof(char) * 100);
    fgets(str, 100, stdin);
    //remove newline
    str[strlen(str) - 1] = 0;
    return createStrThing(runtime, str, 0);
}
