#include <stdlib.h>
#include <string.h>

#include "main/parse.h"
#include "main/validate.h"
#include "main/transform.h"
#include "main/bytecode.h"
#include "main/codegen.h"
#include "main/execute.h"
#include "main/thing.h"
#include "main/top.h"

#include "test/tests.h"

#define UNUSED(x) (void)(x)

uint8_t checkInt(RetVal ret, int32_t num) {
    if(isRetValError(ret)) {
        return 0;
    } else if(typeOfThing(getRetVal(ret)) != THING_TYPE_INT) {
        return 0;
    } else {
        return thingAsInt(getRetVal(ret)) == num;
    }
}

uint8_t checkStr(RetVal ret, const char* str) {
    if(isRetValError(ret)) {
        return 0;
    } else if(typeOfThing(getRetVal(ret)) != THING_TYPE_STR) {
        return 0;
    } else {
        return strcmp(thingAsStr(getRetVal(ret)), str) == 0;
    }
}

uint8_t checkBool(RetVal ret, uint8_t value) {
    if(isRetValError(ret)) {
        return 0;
    } else if(typeOfThing(getRetVal(ret)) != THING_TYPE_BOOL) {
        return 0;
    } else {
        return thingAsBool(getRetVal(ret)) == value;
    }
}

const char* executeTestGlobalHasMainFunc() {
    initThing();
    Runtime* runtime = createRuntime();
    char* errorMsg;
    Module* module = sourceToModule("main = def x do return 1; end;", &errorMsg);
    assert(module != NULL, "error in source code");

    RetVal global = executeModule(runtime, module);
    assert(!isRetValError(global), "error occurred while executing the module");
    assert(typeOfThing(getRetVal(global)) == THING_TYPE_MODULE, "global is not a module");

    Thing* mainFunc = getModuleProperty(getRetVal(global), "main");
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
    in.src = "main = def x do return 1; end;";
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
    in.src = "add_sub = def x y z do return x + y - z; end;";
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
    in.src = "math = def x do return (x*2 - 5) / 7; end;";
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
   in.src = "main = def x do return 'hello world'; end;";
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
    in.src = "greet = def name do return 'hello ' + name + '!'; end;";
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
    in.src = "equal = def x y do return x == y; end;";
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
    in.src = "not_equal = def x y do return x != y; end;";
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
    in.src = "less_than = def x y do return x < y; end;";
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
    in.src = "less_than_eq = def x y do return x <= y; end;";
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
    in.src = "greater_than = def x y do return x > y; end;";
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
    in.src = "greater_than_eq = def x y do return x >= y; end;";
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
    in.src = "str_eq = def x y do return x == y; end;";
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
    in.src = "str_not_eq = def x y do return x != y; end;";
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
    in.src = "bool_and = def x y do return x and y; end;";
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
    in.src = "bool_and = def x y do return x or y; end;";
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
    //TODO make function name consistent
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "bool_and = def x y do return not x; end;";
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
    in.src = "if_stmt = def x y do if x == y then a = 'equal'; else b ='not_equal'; end return a; end;";
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

const char* executeTestAssignment() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "assign = def x do y = 1; return y; end;";
    in.name = "assign";
    in.arity = 1;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = in.runtime->noneThing;

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkInt(out.retVal, 1), "return value is not 1");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestWhileLoop() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "fact = def x do y = 1; while x > 1 do y = y * x; x = x - 1; end return y; end;";
    in.name = "fact";
    in.arity = 1;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 4);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkInt(out.retVal, 24), "return value is not 24");

    cleanupExecFunc(in, out);
    return NULL;
}

RetVal absFunc(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_INT);
    if(isRetValError(retVal)) {
        return retVal;
    }

    int32_t value = thingAsInt(args[0]);
    int32_t out;
    if(value < 0) {
        out = -value;
    } else {
        out = value;
    }
    return createRetVal(createIntThing(runtime, out), 0);
}

const char* executeTestNativeFunc() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "apply = def f x do return f x; end;";
    in.name = "apply";
    in.arity = 2;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createNativeFuncThing(in.runtime, absFunc);
    in.args[1] = createIntThing(in.runtime, -4);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkInt(out.retVal, 4), "return value is not 4");

    cleanupExecFunc(in, out);
    return NULL;
}

const char* executeTestRecFunc() {
    initThing();

    ExecFuncIn in;
    in.runtime = createRuntime();
    in.src = "fact = def x do if x == 1 then return 1; else return x * fact (x - 1); end end;";
    in.name = "fact";
    in.arity = 1;
    in.args = malloc(sizeof(Thing*) * in.arity);
    in.args[0] = createIntThing(in.runtime, 5);

    ExecFuncOut out = execFunc(in);
    assert(out.errorMsg == NULL, out.errorMsg);
    assert(checkInt(out.retVal, 120), "return value is not 120");

    cleanupExecFunc(in, out);
    return NULL;
}
