#include <main/runtime.hpp>
#include <main/thing.h>
#include "main/util.h"

Thing* initOperatorsModule(Runtime* runtime) {
    Map* map = createMap();

    putMapStr(map, "add", getMapStr(runtime->operators, "+"));
    putMapStr(map, "subtract", getMapStr(runtime->operators, "-"));
    putMapStr(map, "multiply", getMapStr(runtime->operators, "*"));
    putMapStr(map, "divide", getMapStr(runtime->operators, "/"));
    putMapStr(map, "equal", getMapStr(runtime->operators, "=="));
    putMapStr(map, "not_equal", getMapStr(runtime->operators, "!="));
    putMapStr(map, "less_than", getMapStr(runtime->operators, "<"));
    putMapStr(map, "less_than_equal", getMapStr(runtime->operators, "<="));
    putMapStr(map, "more_than", getMapStr(runtime->operators, ">"));
    putMapStr(map, "more_than_equal", getMapStr(runtime->operators, ">="));
    putMapStr(map, "op_and", getMapStr(runtime->operators, "and"));
    putMapStr(map, "op_or", getMapStr(runtime->operators, "or"));
    putMapStr(map, "op_not", getMapStr(runtime->operators, "not"));
    putMapStr(map, "cons", getMapStr(runtime->operators, "::"));
    putMapStr(map, "access", getMapStr(runtime->operators, "."));
    putMapStr(map, "call", createSymbolThing(runtime, SYM_CALL, 0));
    putMapStr(map, "unpack", createSymbolThing(runtime, SYM_UNPACK, 2));

    Thing* module = createModuleThing(runtime, map);
    destroyMap(map, nothing, nothing);
    return module;
}
