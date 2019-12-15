#include <stdlib.h>

#include "main/parse.h"
#include "main/validate.h"
#include "main/transform.h"
#include "main/codegen.h"
#include "main/execute.h"
#include "test/tests.h"

Module* sourceToModule(const char* src) {
    BlockToken* ast = parseModule(src);
    if(ast == NULL) {
        return NULL;
    }
    if(!validateModule(ast)) {
        return NULL;
    }
    BlockToken* transformed = transformModule(ast);
    destroyToken((Token*) ast);
    Module* module = compileModule((Token*) transformed);
    destroyToken((Token*) transformed);
    return module;
}

const char* executeTestGlobalHasMainFunc() {
    initExecute();
    Runtime* runtime = createRuntime();
    int error = 0;
    Module* module = sourceToModule("def main x do <- 1; end");
    assert(module != NULL, "error in source code");

    Thing* global = executeModule(runtime, module, &error);
    assert(!error, "error occurred while executing the module");
    assert(typeOfThing(global) == THING_TYPE_OBJ, "global is not an object");

    Thing* mainFunc = getObjectProperty(global, "main");
    assert(mainFunc != NULL, "main function not found");
    assert(typeOfThing(mainFunc) == THING_TYPE_FUNC, "main is not a function");

    destroyRuntime(runtime);
    destroyModule(module);
    deinitExecute();
    return NULL;
}
