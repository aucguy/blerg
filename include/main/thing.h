#ifndef THING_H_
#define THING_H_

#include <stdlib.h>
#include <stdint.h>

#include "main/execute.h"
#include "main/runtime.h"

/**
 * Initialize this module. Must be called before any other functions in this
 * module are called.
 */
void initThing();

/**
 * Deinitialize this module. Must be called or else memleaks will occur. Once
 * called, no other functions in this module may be called until initThing is
 * called.
 */
void deinitThing();

//different builtin types. Initialized in initThing.
extern ThingType* THING_TYPE_NONE;
extern ThingType* THING_TYPE_INT;
extern ThingType* THING_TYPE_FLOAT;
extern ThingType* THING_TYPE_STR;
extern ThingType* THING_TYPE_BOOL;
extern ThingType* THING_TYPE_SYMBOL;
extern ThingType* THING_TYPE_MODULE;
extern ThingType* THING_TYPE_FUNC;
extern ThingType* THING_TYPE_NATIVE_FUNC;
extern ThingType* THING_TYPE_ERROR;
extern ThingType* THING_TYPE_TUPLE;
extern ThingType* THING_TYPE_LIST;
extern ThingType* THING_TYPE_OBJECT;
extern ThingType* THING_TYPE_CELL;

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
Thing* createFloatThing(Runtime* runtime, float value);

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
 * Gets the value associated with the given name in the given ModuleThing. If
 * the thing is not an ModuleThing, this results in undefined behavior.
 */
Thing* getModuleProperty(Thing* thing, const char* name);

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

extern uint32_t SYM_ADD;
extern uint32_t SYM_SUB;
extern uint32_t SYM_MUL;
extern uint32_t SYM_DIV;
extern uint32_t SYM_EQ;
extern uint32_t SYM_NOT_EQ;
extern uint32_t SYM_LESS_THAN;
extern uint32_t SYM_LESS_THAN_EQ;
extern uint32_t SYM_GREATER_THAN;
extern uint32_t SYM_GREATER_THAN_EQ;
extern uint32_t SYM_AND;
extern uint32_t SYM_OR;
extern uint32_t SYM_NOT;
extern uint32_t SYM_GET;
extern uint32_t SYM_DOT;
extern uint32_t SYM_CALL;
extern uint32_t SYM_RESPONDS_TO;
extern uint32_t SYM_UNPACK;

Thing* createSymbolThing(Runtime* runtime, uint32_t id, uint8_t arity);
uint32_t newSymbolId();
uint32_t getSymbolId(Thing* symbol);

Thing* createFuncThing(Runtime* runtime, uint32_t entry,
        Module* module, Scope* parentScope);

Thing* createModuleThing(Runtime* runtime, Map* map);

Thing* createNativeFuncThing(Runtime* runtime, ExecFunc func);

Thing* createErrorThing(Runtime* runtime, const char* msg);
const char* errorStackTrace(Runtime* runtime, Thing* self);

Thing* createTupleThing(Runtime* runtime, uint8_t size, Thing** elements);
uint8_t getTupleSize(Thing* tuple);
Thing* getTupleElem(Thing* tuple, uint8_t index);

typedef struct {
    Thing* head;
    Thing* tail;
} ListThing;

Thing* createListThing(Runtime* runtime, Thing* head, Thing* tail);

Thing* createObjectThing(Runtime* runtime, Map* map);
Map* getObjectMap(Thing* object);

Thing* createCellThing(Runtime* runtime, Thing* value);
Thing* getCellValue(Thing* cell);
void setCellValue(Thing* cell, Thing* value);

RetVal typeCheck(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t expectedArity, ...);

void destroySimpleThing(Thing* thing);
RetVal errorCall(Runtime* runtime, Thing* thing, Thing** args, uint8_t arity);
Thing* createThing(Runtime* runtime, ThingType* type, size_t size);
RetVal symbolDispatch(Runtime*, Thing*, Thing**, uint8_t);

#endif /* THING_H_ */
