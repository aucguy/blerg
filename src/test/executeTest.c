#include <stdlib.h>

#include "main/parse.h"
#include "main/validate.h"
#include "main/transform.h"
#include "main/bytecode.h"
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

const char* executeTestMainFuncReturns1() {
    initExecute();
    Runtime* runtime = createRuntime();
    int error = 0;
    Module* module = sourceToModule("def main x do <- 1; end");
    assert(module != NULL, "error in source code");

    Thing* global = executeModule(runtime, module, &error);
    Thing* mainFunc = getObjectProperty(global, "main");
    Thing** args = malloc(sizeof(Thing*) * 1);
    args[0] = runtime->noneThing;
    Thing* retVal = callFunction(runtime, mainFunc, 1, args, &error);
    assert(error == 0 , "failed to execute the function");

    assert(typeOfThing(retVal) == THING_TYPE_INT, "return value is not an integer");
    assert(thingAsInt(retVal) == 1, "return value is not 1");

    free(args);
    destroyModule(module);
    destroyRuntime(runtime);
    deinitExecute();
    return NULL;
}

const char* executeTestAddFunction() {
    initExecute();
    Runtime* runtime = createRuntime();
    int error = 0;
    Module* module = sourceToModule("def add x y do <- x + y; end");
    assert(module != NULL, "error in source code");

    Thing* global = executeModule(runtime, module, &error);
    assert(error == 0, "error executing global scope");
    assert(typeOfThing(global) == THING_TYPE_OBJ, "global scope is not an object");
    Thing* addFunc = getObjectProperty(global, "add");
    assert(addFunc != NULL, "add function not found")
    assert(typeOfThing(addFunc) == THING_TYPE_FUNC, "add function is not a function");

    Thing** args = malloc(sizeof(Thing*) * 2);
    args[0] = createIntThing(runtime, 1);
    args[1] = createIntThing(runtime, 2);
    Thing* retVal = callFunction(runtime, addFunc, 2, args, &error);
    assert(error == 0, "error executing add function");
    assert(typeOfThing(retVal) == THING_TYPE_INT, "returned value is not an integer");
    assert(thingAsInt(retVal) == 3, "returned value is not 3");

    destroyRuntime(runtime);
    destroyModule(module);
    free(args);
    deinitExecute();
    return NULL;
}
