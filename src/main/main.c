#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main/flags.h"
#include "main/top.h"

#if INCLUDE_TESTS
#include "test/tests.h"
#endif

/*typedef struct {
    Runtime* runtime;
    const char* src;
    const char* name;
    Thing** args;
    uint8_t arity;
} ExecFuncIn;

typedef struct {
    Thing* retVal;
    const char* errorMsg;
    Module* module;
} ExecFuncOut;

//in executeTest.c
ExecFuncOut execFunc(ExecFuncIn in);*/

int main(int argc, const char* args[]) {
#if INCLUDE_TESTS
    if(argc >= 2 && strcmp(args[1], "--test") == 0) {
        return runTests();
    }
#endif

    if(argc == 2) {
        initThing();
        ExecFuncIn in;
        in.runtime = createRuntime();
        in.src = readFile(args[1]);
        in.name = "main";
        in.arity = 1;
        in.args = malloc(sizeof(Thing*) * in.arity);
        in.args[0] = in.runtime->noneThing;
        ExecFuncOut out = execFunc(in);
        if(out.errorMsg != NULL) {
            printf("error: %s", out.errorMsg);
        }
        free((char*) in.src);
        cleanupExecFunc(in, out);
    }
    return 0;
}
