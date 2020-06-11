#include <string.h>

#include "main/util.h"
#include "main/thing.h"
#include "main/execute.h"

#include "main/std_lib/functools.h"
#include "main/std_lib/operators.h"
#include "main/std_lib/internal/inheritance.h"

Thing* loadBuiltinModule(Runtime* runtime, const char* filename) {
    if(strcmp(filename, "std/builtin_test.blg") == 0) {
        Map* map = createMap();
        putMapStr(map, "hello", createStrThing(runtime, "world", 1));
        Thing* module = createModuleThing(runtime, map);
        destroyMap(map, nothing, nothing);
        return module;
    } else if(strcmp(filename, "std/functools.blg") == 0) {
        return initFunctoolsModule(runtime);
    } else if(strcmp(filename, "std/operators.blg") == 0) {
        return initOperatorsModule(runtime);
    } else if(strcmp(filename, "std/internal/inheritance.blg") == 0) {
        return initInheritanceModule(runtime);
    } else {
        return NULL;
    }
}

void destroyBuiltinModules() {
    destroyFunctoolsModule();
}
