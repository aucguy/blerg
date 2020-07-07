#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main/flags.h"
#include "main/top.h"

#if INCLUDE_TESTS
#include "test/tests.h"
#endif

int main(int argc, const char* args[]) {
#if INCLUDE_TESTS
    if(argc >= 2 && strcmp(args[1], "--test") == 0) {
        return runTests(argc, args);
    }
#endif

    if(argc == 2) {
        initThing();
        ExecFuncIn in;
        in.runtime = createRuntime(argc, args);
        in.src = readFile(args[1]);
        in.name = "main";
        in.arity = 1;
        in.args = (Thing**) malloc(sizeof(Thing*) * in.arity);
        in.args[0] = in.runtime->noneThing;
        in.filename = args[1];
        ExecFuncOut out = execFunc(in);
        if(out.errorMsg != NULL) {
            printf("error: %s", out.errorMsg);
        }
        free((char*) in.src);
        cleanupExecFunc(in, out);
    }
    return 0;
}
