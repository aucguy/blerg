#include <stdlib.h>
#include <string.h>

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
        out.errorMsg = "error executing function";
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
    } else {
        return thingAsInt(thing) == num;
    }
}

uint8_t checkStr(Thing* thing, const char* str) {
    if(typeOfThing(thing) != THING_TYPE_STR) {
        return 0;
    } else {
        return strcmp(thingAsStr(thing), str) == 0;
    }
}

uint8_t checkBool(Thing* thing, uint8_t value) {
    if(typeOfThing(thing) != THING_TYPE_BOOL) {
        return 0;
    } else {
        return thingAsBool(thing) == value;
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

const char* executeTestMathExpr() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def math x do <- (x*2 - 5) / 7; end";
    in.name = "math";
    in.arity = 1;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 100);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkInt(out.retVal, 27), "returned value is not 27");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestStrRet() {
   initThing();

   ExecFuncIn in;
   in.runtime = createRuntime();
   in.src = "def main x do <- 'hello world'; end";
   in.name = "main";
   in.arity = 1;
   in.args = malloc(sizeof(Thing*) * in.arity);
   in.args[0] = in.runtime->noneThing;

   ExecFuncOut out = execFunc(in);
   assert(out.errorMsg == NULL, out.errorMsg);
   assert(checkStr(out.retVal, "hello world"), "return value is not 'hello world'");

   cleanupExecFunc(in, out);
   return NULL;
}

const char* executeTestStrConcat() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def greet name do <- 'hello ' + name + '!'; end";
    in.name = "greet";
    in.arity = 1;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createStrThing(in.runtime, "Bob", 1);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkStr(out.retVal, "hello Bob!"), "return value is not 'hello Bob!'");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestIntEq() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def equal x y do <- x == y; end";
    in.name = "equal";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 5);
    in.args[1] = createIntThing(in.runtime, 5);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 1), "return value is not True");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestIntNotEq() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def not_equal x y do <- x != y; end";
    in.name = "not_equal";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 5);
    in.args[1] = createIntThing(in.runtime, 5);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 0), "return value is not False");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestIntLessThan() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def less_than x y do <- x < y; end";
    in.name = "less_than";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 3);
    in.args[1] = createIntThing(in.runtime, 5);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 1), "return value is not True");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestIntLessThanEq() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def less_than_eq x y do <- x <= y; end";
    in.name = "less_than_eq";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 3);
    in.args[1] = createIntThing(in.runtime, 5);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 1), "return value is not True");

    cleanupExecFunc(in, out);
    return NULL;
}
const char* executeTestIntGreaterThan() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def greater_than x y do <- x > y; end";
    in.name = "greater_than";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 3);
    in.args[1] = createIntThing(in.runtime, 5);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 0), "return value is not False");

    cleanupExecFunc(in, out);
    return NULL;
}
const char* executeTestIntGreaterThanEq() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def greater_than_eq x y do <- x >= y; end";
    in.name = "greater_than_eq";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 3);
    in.args[1] = createIntThing(in.runtime, 5);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 0), "return value is not False");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestStrEq() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def str_eq x y do <- x == y; end";
    in.name = "str_eq";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createStrThing(in.runtime, "hello", 1);
    in.args[1] = createStrThing(in.runtime, "world", 1);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 0), "return value is not False");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestStrNotEq() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def str_not_eq x y do <- x != y; end";
    in.name = "str_not_eq";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createStrThing(in.runtime, "hello", 1);
    in.args[1] = createStrThing(in.runtime, "world", 1);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 1), "return value is not True");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestBoolAnd() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def bool_and x y do <- x and y; end";
    in.name = "bool_and";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createBoolThing(in.runtime, 1);
    in.args[1] = createBoolThing(in.runtime, 0);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 0), "return value is not False");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestBoolOr() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def bool_and x y do <- x or y; end";
    in.name = "bool_and";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createBoolThing(in.runtime, 1);
    in.args[1] = createBoolThing(in.runtime, 0);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 1), "return value is not True");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestBoolNot() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def bool_and x y do <- not x; end";
    in.name = "bool_and";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createBoolThing(in.runtime, 1);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkBool(out.retVal, 0), "return value is not False");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestIfStmt() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "def if_stmt x y do if x == y then <- 'equal'; else <- 'not_equal'; end end";
    in.name = "if_stmt";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 1);
    in.args[1] = createIntThing(in.runtime, 1);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkStr(out.retVal, "equal"), "return value is not 'equal'");

    cleanupExecFunc(in, out);
    return NULL;
}
