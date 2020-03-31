#ifndef THING_H_
#define THING_H_

#include <stdint.h>

typedef struct Runtime Runtime;
typedef struct Scope_ Scope;
typedef void Thing;

/**
 * Initialize this module. Must be called before any other functions in this
 * module are called.
 */
void initThing();

/**
 * Deinitialize this module. Must be called or else memleaks will occur. Once
 * called, no other functions in this module may be called until initExecute is
 * called.
 */
void deinitThing();

typedef Thing* (*ExecFunc)(Runtime*, Thing*, Thing**, uint8_t, uint8_t*);

/**
 * Each Thing has a type which describes how it behaves and its custom data
 * format.
 */
typedef struct {
    //destroys the thing. Should not destroy any references this thing has to
    //other things.
    void (*destroy)(Thing*);
    ExecFunc call;
    ExecFunc dispatch;
} ThingType;

//different builtin types. Initialized in initThing.
ThingType* THING_TYPE_NONE;
ThingType* THING_TYPE_INT;
ThingType* THING_TYPE_STR;
ThingType* THING_TYPE_BOOL;
ThingType* THING_TYPE_SYMBOL;
ThingType* THING_TYPE_OBJ;
ThingType* THING_TYPE_FUNC;

/**
 * Describes an 'object' within the blerg program.
 */
typedef struct {
    ThingType* type;
} ThingHeader;

Thing* thingHeaderToCustomData(ThingHeader* header);
ThingHeader* customDataToThingHeader(Thing* thing);

void destroyThing(Thing* thing);

Thing* createIntThing(Runtime* runtime, int32_t value);

Thing* createStrThing(Runtime* runtime, const char* value, uint8_t literal);

Thing* createBoolThing(Runtime* runtime, uint8_t value);
uint8_t thingAsBool(Thing* thing);

/**
 * Returns the integer value of the given IntThing. If the thing is not an
 * IntThing, this results in undefined behavior.
 */
int32_t thingAsInt(Thing* thing);

const char* thingAsStr(Thing* thing);

/**
 * Gets the value associated with the given name in the given ObjectThing. If
 * the thing is not an ObjectThing, this results in undefined behavior.
 */
Thing* getObjectProperty(Thing* thing, const char* name);

/**
 * Returns the type of the given thing. Use this to check its type before
 * passing it to functions which require a certain type.
 */
ThingType* typeOfThing(Thing* thing);

typedef struct {
    //location of first bytecode of the function
    unsigned int entry;
    //module the function was declared in
    Module* module;
    //the scope the function was declared in
    Scope* parentScope;
} FuncThing;

Thing* createNoneThing(Runtime* runtime);

uint32_t SYM_ADD;
uint32_t SYM_SUB;
uint32_t SYM_MUL;
uint32_t SYM_DIV;
uint32_t SYM_EQ;
uint32_t SYM_NOT_EQ;
uint32_t SYM_LESS_THAN;
uint32_t SYM_LESS_THAN_EQ;
uint32_t SYM_GREATER_THAN;
uint32_t SYM_GREATER_THAN_EQ;
uint32_t SYM_ADD;
uint32_t SYM_OR;
uint32_t SYM_NOT;

Thing* createSymbolThing(Runtime* runtime, uint32_t id, uint8_t arity);
uint32_t newSymbolId();

Thing* createFuncThing(Runtime* runtime, uint32_t entry,
        Module* module, Scope* parentScope);

Thing* createObjectThingFromMap(Runtime* runtime, Map* map);

#endif /* THING_H_ */
