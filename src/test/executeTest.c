#include <stdlib.h>

#include "main/parse.h"
#include "main/validate.h"
#include "main/transform.h"
#include "main/bytecode.h"
#include "main/codegen.h"
#include "main/execute.h"
#include "main/thing.h"

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

typedef struct {
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

ExecFuncOut execFunc(ExecFuncIn in) {
    ExecFuncOut out;
    out.retVal = NULL;
    out.errorMsg = NULL;
    out.module = NULL;

    uint8_t error = 0;
    out.module = sourceToModule(in.src);
    if(out.module == NULL) {
        out.errorMsg = "error in source code";
        return out;
    }

    Thing* global = executeModule(in.runtime, out.module, &error);
    if(error != 0) {
        out.errorMsg = "error executing global scope";
        return out;
    } else if(typeOfThing(global) != THING_TYPE_OBJ) {
        out.errorMsg = "global scope is not an object";
        return out;
    }

    Thing* func = getObjectProperty(global, in.name);
    if(func == NULL) {
        out.errorMsg = "function not found";
        return out;
    } else if(typeOfThing(func) != THING_TYPE_FUNC) {
        out.errorMsg = "function is not a function";
        return out;
    }

    out.retVal = callFunction(in.runtime, func, in.arity, in.args, &error);
    if(error != 0) {
        out.errorMsg = "error executing add function";
        return out;
    }
    return out;
}

void cleanupExecFunc(ExecFuncIn in, ExecFuncOut out) {
    destroyRuntime(in.runtime);
    destroyModule(out.module);
    free(in.args);
    deinitThing();
}

uint8_t checkInt(Thing* thing, int32_t num) {
    if(typeOfThing(thing) != THING_TYPE_INT) {
        return 0;
    } else if(thingAsInt(thing) != num) {
        return 0;
    } else {
        return 1;
    }
}

const char* executeTestGlobalHasMainFunc() {
    initThing();
    Runtime* runtime = createRuntime();
    uint8_t error = 0;
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
    deinitThing();
    return NULL;
}

const char* executeTestMainFuncReturns1() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def main x do <- 1; end";
    in.name = "main";
    in.arity = 1;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = in.runtime->noneThing;

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkInt(out.retVal, 1), "return value is not 1");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestAddSubFunction() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def add_sub x y z do <- x + y - z; end";
    in.name = "add_sub";
    in.arity = 3;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 1);
    in.args[1] = createIntThing(in.runtime, 2);
    in.args[2] = createIntThing(in.runtime, 3);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkInt(out.retVal, 0), "returned value is not 0");

    cleanupExecFunc(in, out);
    return NULL;
}
