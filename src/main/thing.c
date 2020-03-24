#include <stdlib.h>

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

void setCallThingType(ThingType* type, Thing* (*call)(Runtime*, Thing*, Thing**, int*)) {
    type->call = call;
}

void setArityThingType(ThingType* type, unsigned char (*arity)(Thing*)) {
    type->arity = arity;
}

void destroySimpleThing(Thing* thing) {
    free(thing);
}

Thing* errorCall(Runtime* runtime, Thing* thing, Thing** args, int* error) {
    UNUSED(runtime);
    UNUSED(thing);
    UNUSED(args);
    *error = 1;
    return NULL;
}

unsigned char arityOne(Thing* thing) {
    UNUSED(thing);
    return 1;
}

unsigned char arityTwo(Thing* thing) {
    UNUSED(thing);
    return 2;
}

Thing* symbolCall(Runtime* runtime, Thing* self, Thing** args, int* error);

void destroyObjectThing(Thing* thing);

static int initialized = 0;

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

ThingType* typeOfThing(Thing* thing) {
    return thing->type;
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
    Thing* thing = malloc(sizeof(Thing) + size);
    thing->type = type;
    runtime->allocatedThings = consList(thing, runtime->allocatedThings);
    return thing;
}

/**
 * Returns the custom data associated with the thing. The size is the same
 * passed to createThing.
 */
void* thingCustomData(Thing* thing) {
    return ((char*) thing) + sizeof(Thing);
}

/**
 * Singleton none object.
 */
typedef struct {
    //structs must have nonzero length.
    char dummy;
} NoneThing;

Thing* createNoneThing(Runtime* runtime) {
    Thing* thing = createThing(runtime, THING_TYPE_NONE, sizeof(NoneThing));
    NoneThing* noneThing = thingCustomData(thing);
    noneThing->dummy = 0;
    return thing;
}

/**
 * Represents integers found in the source code.
 */
typedef struct {
    int value;
} IntThing;

Thing* createIntThing(Runtime* runtime, int value) {
    Thing* thing = createThing(runtime, THING_TYPE_INT, sizeof(IntThing));
    IntThing* intThing = thingCustomData(thing);
    intThing->value = value;
    return thing;
}

int thingAsInt(Thing* thing) {
    IntThing* data = thingCustomData(thing);
    return data->value;
}

int symbolId = 0;

int newSymbolId() {
    return symbolId++;
}

typedef struct {
    int id;
} SymbolThing;

Thing* createSymbolThing(Runtime* runtime, int id) {
    Thing* thing = createThing(runtime, THING_TYPE_SYMBOL, sizeof(SymbolThing));
    SymbolThing* symbolThing = thingCustomData(thing);
    symbolThing->id = id;
    return thing;
}


//not properly supported yet
Thing* symbolCall(Runtime* runtime, Thing* self, Thing** args, int* error) {
    UNUSED(self);
    //only ints supported
    if(typeOfThing(args[0]) != THING_TYPE_INT || typeOfThing(args[1]) != THING_TYPE_INT) {
        *error = 1;
        return NULL;
    }

    int valueA = thingAsInt(args[0]);
    int valueB = thingAsInt(args[1]);

    return (Thing*) createIntThing(runtime, valueA + valueB);
}

/**
 * Used for module objects and object literals. Associates strings with things.
 */
typedef struct {
    Map* properties;
} ObjectThing;

Thing* createObjectThingFromMap(Runtime* runtime, Map* map) {
    Thing* thing = createThing(runtime, THING_TYPE_OBJ, sizeof(ObjectThing));
    ObjectThing* objThing = thingCustomData(thing);
    objThing->properties = copyMap(map);
    return thing;
}

void destroyObjectThing(Thing* thing) {
    ObjectThing* objThing = thingCustomData(thing);
    destroyMap(objThing->properties, nothing, nothing);
    free(thing);
}

Thing* getObjectProperty(Thing* thing, const char* name) {
    ObjectThing* objThing = (ObjectThing*) thingCustomData(thing);
    return getMapStr(objThing->properties, name);
}

Thing* createFuncThing(Runtime* runtime, unsigned int entry,
        Module* module, Scope* parentScope) {
    Thing* thing = createThing(runtime, THING_TYPE_FUNC, sizeof(FuncThing));
    FuncThing* funcThing = thingCustomData(thing);
    funcThing->entry = entry;
    funcThing->module = module;
    funcThing->parentScope = parentScope;
    return thing;
}
