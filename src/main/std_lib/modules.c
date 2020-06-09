#include <string.h>

#include "main/util.h"
#include "main/thing.h"
#include "main/execute.h"

#include "main/std_lib/functools.h"

Thing* loadBuiltinModule(Runtime* runtime, const char* filename) {
    if(strcmp(filename, "std/builtin_test.blg") == 0) {
        Map* map = createMap();
        putMapStr(map, "hello", createStrThing(runtime, "world", 1));
        Thing* module = createModuleThing(runtime, map);
        destroyMap(map, nothing, nothing);
        return module;
    } else if(strcmp(filename, "std/functools.blg") == 0) {
        return initFunctoolsModule(runtime);
    } else {
        return NULL;
    }
}
