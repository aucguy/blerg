#include <stdlib.h>
#include <stdint.h>

#include "main/execute.h"
#include "main/thing.h"

#define UNUSED(x) (void)(x)

ThingType* createThingType() {
    ThingType* type = malloc(sizeof(ThingType));
    type->destroy = NULL;
    return type;
}

void setDestroyThingType(ThingType* type, void (*destroy)(Thing*)) {
    type->destroy = destroy;
}

void setCallThingType(ThingType* type, ExecFunc call) {
    type->call = call;
}

void setDispatchThingType(ThingType* type, ExecFunc dispatch) {
    type->dispatch = dispatch;
}

void destroySimpleThing(Thing* thing) {
    UNUSED(thing);
}

Thing* errorCall(Runtime* runtime, Thing* thing, Thing** args, uint8_t arity,
        uint8_t* error) {
    UNUSED(runtime);
    UNUSED(thing);
    UNUSED(arity);
    UNUSED(args);
    *error = 1;
    return NULL;
}

Thing* symbolCall(Runtime*, Thing*, Thing**, uint8_t, uint8_t*);
Thing* intDispatch(Runtime*, Thing*, Thing**, uint8_t, uint8_t*);
uint32_t getSymbolId(Thing* self);

void destroyObjectThing(Thing* thing);

static uint8_t initialized = 0;

void initThing() {
    if(!initialized) {
        THING_TYPE_NONE = createThingType();
        setDestroyThingType(THING_TYPE_NONE, destroySimpleThing);
        setCallThingType(THING_TYPE_NONE, errorCall);
        setDispatchThingType(THING_TYPE_NONE, errorCall);

        THING_TYPE_SYMBOL = createThingType();
        setDestroyThingType(THING_TYPE_SYMBOL, destroySimpleThing);
        setCallThingType(THING_TYPE_SYMBOL, symbolCall);
        setDispatchThingType(THING_TYPE_SYMBOL, symbolCall);

        THING_TYPE_INT = createThingType();
        setDestroyThingType(THING_TYPE_INT, destroySimpleThing);
        setCallThingType(THING_TYPE_INT, errorCall);
        setDispatchThingType(THING_TYPE_INT, intDispatch);

        THING_TYPE_OBJ = createThingType();
        setDestroyThingType(THING_TYPE_OBJ, destroyObjectThing);
        setCallThingType(THING_TYPE_OBJ, errorCall);
        setDispatchThingType(THING_TYPE_OBJ, errorCall);

        THING_TYPE_FUNC = createThingType();
        setDestroyThingType(THING_TYPE_FUNC, destroySimpleThing);
        setCallThingType(THING_TYPE_FUNC, errorCall);
        setDispatchThingType(THING_TYPE_FUNC, errorCall);

        SYM_ADD = newSymbolId();
        SYM_SUB = newSymbolId();
        SYM_MUL = newSymbolId();
        SYM_DIV = newSymbolId();

        initialized = 1;
    }
}

void deinitThing() {
    if(initialized) {
        free(THING_TYPE_NONE);
        THING_TYPE_NONE = NULL;

        free(THING_TYPE_INT);
        THING_TYPE_INT = NULL;

        free(THING_TYPE_SYMBOL);
        THING_TYPE_SYMBOL = NULL;

        free(THING_TYPE_OBJ);
        THING_TYPE_OBJ = NULL;

        free(THING_TYPE_FUNC);
        THING_TYPE_FUNC = NULL;

        SYM_ADD = 0;
        SYM_SUB = 0;
        SYM_MUL = 0;
        SYM_DIV = 0;

        initialized = 0;
    }
}

Thing* thingHeaderToCustomData(ThingHeader* header) {
    return ((char*) header) + sizeof(ThingHeader);
}

ThingHeader* customDataToThingHeader(Thing* thing) {
    return (ThingHeader*) (((char*) thing) - sizeof(ThingHeader));
}

/**
 * Creates a thing. If the runtime is destroyed, the object will also be
 * destroyed.
 *
 * @param runtime the runtime object
 * @param type the native type of the object
 * @param size the size of the object's custom data. The custom data is found
 *      after the Thing struct.
 */
Thing* createThing(Runtime* runtime, ThingType* type, size_t size) {
    ThingHeader* header = malloc(sizeof(ThingHeader) + size);
    header->type = type;
    Thing* thing = thingHeaderToCustomData(header);
    runtime->allocatedThings = consList(thing, runtime->allocatedThings);
    return thing;
}

void destroyThing(Thing* thing) {
    ThingHeader* header = customDataToThingHeader(thing);
    header->type->destroy((Thing*) thing);
    free(header);
}

ThingType* typeOfThing(Thing* thing) {
    return customDataToThingHeader(thing)->type;
}

/**
 * Singleton none object.
 */
typedef struct {
    //structs must have nonzero length.
    char dummy;
} NoneThing;

Thing* createNoneThing(Runtime* runtime) {
    NoneThing* thing = createThing(runtime, THING_TYPE_NONE, sizeof(NoneThing));
    thing->dummy = 0;
    return thing;
}

/**
 * Represents integers found in the source code.
 */
typedef struct {
    int32_t value;
} IntThing;

Thing* createIntThing(Runtime* runtime, int32_t value) {
    IntThing* thing = createThing(runtime, THING_TYPE_INT, sizeof(IntThing));
    thing->value = value;
    return thing;
}

Thing* intDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error) {
    UNUSED(self);
    if(arity != 2 || typeOfThing(args[0]) != THING_TYPE_INT ||
            typeOfThing(args[1]) != THING_TYPE_INT ||
            typeOfThing(self) != THING_TYPE_SYMBOL) {
        *error = 1;
        return NULL;
    }

    int32_t valueA = thingAsInt(args[0]);
    int32_t valueB = thingAsInt(args[1]);
    int32_t out;
    uint32_t id = getSymbolId(self);

    if(id == SYM_ADD) {
        out = valueA + valueB;
    } else if(id == SYM_SUB) {
        out = valueA - valueB;
    } else if(id == SYM_MUL) {
        out = valueA * valueB;
    } else if(id == SYM_DIV) {
        out = valueA / valueB;
    } else {
        *error = 1;
        return NULL;
    }

    return (Thing*) createIntThing(runtime, out);
}

int32_t thingAsInt(Thing* thing) {
    return ((IntThing*) thing)->value;
}

uint32_t symbolId = 0;

uint32_t newSymbolId() {
    return symbolId++;
}

typedef struct {
    uint32_t id;
    uint8_t arity;
} SymbolThing;

Thing* createSymbolThing(Runtime* runtime, uint32_t id, uint8_t arity) {
    SymbolThing* thing = createThing(runtime, THING_TYPE_SYMBOL,
            sizeof(SymbolThing));
    thing->id = id;
    thing->arity = arity;
    return thing;
}

uint32_t getSymbolId(Thing* self) {
    return ((SymbolThing*) self)->id;
}

//not properly supported yet
Thing* symbolCall(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error) {
    if(arity != ((SymbolThing*) self)->arity) {
        *error = 1;
        return NULL;
    }
    return typeOfThing(args[0])->dispatch(runtime, self, args, arity, error);
}

/**
 * Used for module objects and object literals. Associates strings with things.
 */
typedef struct {
    Map* properties;
} ObjectThing;

Thing* createObjectThingFromMap(Runtime* runtime, Map* map) {
    ObjectThing* thing = createThing(runtime, THING_TYPE_OBJ,
            sizeof(ObjectThing));
    thing->properties = copyMap(map);
    return thing;
}

void destroyObjectThing(Thing* thing) {
    destroyMap(((ObjectThing*) thing)->properties, nothing, nothing);
}

Thing* getObjectProperty(Thing* thing, const char* name) {
    return getMapStr(((ObjectThing*) thing)->properties, name);
}

Thing* createFuncThing(Runtime* runtime, uint32_t entry,
        Module* module, Scope* parentScope) {
    FuncThing* thing = createThing(runtime, THING_TYPE_FUNC, sizeof(FuncThing));
    thing->entry = entry;
    thing->module = module;
    thing->parentScope = parentScope;
    return thing;
}
