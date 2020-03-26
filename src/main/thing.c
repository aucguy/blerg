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

void setCallThingType(ThingType* type, Thing* (*call)(Runtime*, Thing*, Thing**, uint8_t*)) {
    type->call = call;
}

void setArityThingType(ThingType* type, uint8_t (*arity)(Thing*)) {
    type->arity = arity;
}

void destroySimpleThing(Thing* thing) {
    UNUSED(thing);
}

Thing* errorCall(Runtime* runtime, Thing* thing, Thing** args, uint8_t* error) {
    UNUSED(runtime);
    UNUSED(thing);
    UNUSED(args);
    *error = 1;
    return NULL;
}

uint8_t arityOne(Thing* thing) {
    UNUSED(thing);
    return 1;
}

uint8_t arityTwo(Thing* thing) {
    UNUSED(thing);
    return 2;
}

Thing* symbolCall(Runtime* runtime, Thing* self, Thing** args, uint8_t* error);

void destroyObjectThing(Thing* thing);

static uint8_t initialized = 0;

void initThing() {
    if(!initialized) {
        THING_TYPE_NONE = createThingType();
        setDestroyThingType(THING_TYPE_NONE, destroySimpleThing);
        setCallThingType(THING_TYPE_NONE, errorCall);
        setArityThingType(THING_TYPE_NONE, arityOne);

        THING_TYPE_SYMBOL = createThingType();
        setDestroyThingType(THING_TYPE_SYMBOL, destroySimpleThing);
        setCallThingType(THING_TYPE_SYMBOL, symbolCall);
        setArityThingType(THING_TYPE_SYMBOL, arityTwo);

        THING_TYPE_INT = createThingType();
        setDestroyThingType(THING_TYPE_INT, destroySimpleThing);
        setCallThingType(THING_TYPE_INT, errorCall);
        setArityThingType(THING_TYPE_INT, arityOne);

        THING_TYPE_OBJ = createThingType();
        setDestroyThingType(THING_TYPE_OBJ, destroyObjectThing);
        setCallThingType(THING_TYPE_OBJ, errorCall);
        setArityThingType(THING_TYPE_OBJ, arityOne);

        THING_TYPE_FUNC = createThingType();
        setDestroyThingType(THING_TYPE_FUNC, destroySimpleThing);
        setCallThingType(THING_TYPE_FUNC, errorCall);
        setArityThingType(THING_TYPE_FUNC, arityOne);

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

int32_t thingAsInt(Thing* thing) {
    return ((IntThing*) thing)->value;
}

uint32_t symbolId = 0;

uint32_t newSymbolId() {
    return symbolId++;
}

typedef struct {
    uint32_t id;
} SymbolThing;

Thing* createSymbolThing(Runtime* runtime, uint32_t id) {
    SymbolThing* thing = createThing(runtime, THING_TYPE_SYMBOL, sizeof(SymbolThing));
    thing->id = id;
    return thing;
}


//not properly supported yet
Thing* symbolCall(Runtime* runtime, Thing* self, Thing** args, uint8_t* error) {
    UNUSED(self);
    //only ints supported
    if(typeOfThing(args[0]) != THING_TYPE_INT || typeOfThing(args[1]) != THING_TYPE_INT) {
        *error = 1;
        return NULL;
    }

    int32_t valueA = thingAsInt(args[0]);
    int32_t valueB = thingAsInt(args[1]);

    return (Thing*) createIntThing(runtime, valueA + valueB);
}

/**
 * Used for module objects and object literals. Associates strings with things.
 */
typedef struct {
    Map* properties;
} ObjectThing;

Thing* createObjectThingFromMap(Runtime* runtime, Map* map) {
    ObjectThing* thing = createThing(runtime, THING_TYPE_OBJ, sizeof(ObjectThing));
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
