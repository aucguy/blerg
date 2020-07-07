#include "main/runtime.h"
#include "main/thing.h"

RetVal libProperties(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    RetVal ret = typeCheck(runtime, self, args, arity, 1, THING_TYPE_OBJECT);
    if(isRetValError(ret)) {
        return ret;
    }

    Thing* list = runtime->noneThing;
    Entry* entry = getObjectMap(args[0])->entry;

    while(entry != NULL) {
        Thing** elements = (Thing**) malloc(sizeof(Thing*) * 2);
        uint32_t* key = (uint32_t*) entry->key;
        elements[0] = createSymbolThing(runtime, *key, 0);
        elements[1] = entry->value;
        Thing* head = createTupleThing(runtime, 2, elements);

        list = createListThing(runtime, head, list);
        entry = entry->tail;
    }

    return createRetVal(list, 0);
}

Thing* initInheritanceModule(Runtime* runtime) {
    Map* map = createMap();

    putMapStr(map, "properties", createNativeFuncThing(runtime, libProperties));
    putMapStr(map, "object", getMapStr(runtime->operators, "object"));

    Thing* module = createModuleThing(runtime, map);
    destroyMap(map, nothing, nothing);
    return module;
}
