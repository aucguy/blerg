#include "main/codegen.h"
#include "main/util.h"

#ifndef EXECUTE_H_
#define EXECUTE_H_

/**
 * Every 'object' in the language is internally referred to as a 'thing.'
 * So there are IntThing, LiteralThing, FuncThing (results of defs) and
 * ObjectThing. Each thing is stored as a Thing struct followed by custom data
 * which is specificed by its type (Int, Literal, Func, Object, etc).
 */

/**
 * Initialize this module. Must be called before any other functions in this
 * module are called.
 */
void initExecute();

/**
 * Deinitalize this module. Must be called or else memleaks will occur. Once
 * called, no other functions in this module may be called until initExecute is
 * called.
 */
void deinitExecute();

typedef struct Thing Thing;

/**
 * Each Thing has a type which describes how it behaves and its custom data
 * format.
 */
typedef struct {
    //destroys the thing. Should not destroy any references this thing has to
    //other things.
    void(*destroy)(struct Thing*);
} ThingType;

//different builtin types. Initialized in initExecute.
ThingType* THING_TYPE_NONE;
ThingType* THING_TYPE_INT;
ThingType* THING_TYPE_OBJ;
ThingType* THING_TYPE_FUNC;

/**
 * Describes an 'object' within the blerg program.
 */
struct Thing {
    ThingType* type;
};

/**
 * The runtime object type. This is a singleton that stores information
 * pertaining to the execution and is used in many different operations.
 */
typedef struct {
    //Stacks are stored as lists. The first item is the top of the stack.
    //List of StackFrames.
    List* stackFrame;
    //List of stack items. There is no designated start or stop for different
    //stackframes. So, different items pertaining to each stackFrame are just
    //stored right after the other.
    List* stack;
    //reference to the singleton NoneThing
    Thing* noneThing;
    //list of things that were allocated. They are deleted once the runtime
    //is destroyed.
    List* allocatedThings;
    //list of allocated scopes. They are deleted once the runtime is destroyed.
    List* allocatedScopes;
} Runtime;

Runtime* createRuntime();
void destroyRuntime(Runtime* runtime);

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

/**
 * Returns the integer value of the given IntThing. If the thing is not an
 * IntThing, this results in undefined behavior.
 */
int thingAsInt(Thing* thing);

/**
 * Executes the given module. The function executes the bytecode at index 0
 * until the module returns. Instead of normally returning the value returned',
 * (the top of the stack), the function returns the scope wrapped in an
 * ObjectThing.
 *
 * @param runtime the runtime object
 * @param module the module whose global scope is to be determined. Execution
 *          begins at the beginning of the bytecode until the first
 *          return instruction.
 * @param error set to a nonzero value if an error occurs during execution.
 * @return the global scope as an ObjectThing, or NULL upon error.
 */
Thing* executeModule(Runtime* runtime, Module* module, int* error);

/**
 * Calls the given function with the given arguments. Currently, partial
 * application is not supported, so the number of arguments supplied must match
 * the arity of the function.
 *
 * @param runtime the runtime object
 * @param thing the function thing to invoke
 * @param argNo the number of arguments
 * @param args an array of Thing* whose length is argNo
 * @param error set to a nonzero value if an error occurs during execution
 * @return the thing returned by the function
 */
Thing* callFunction(Runtime* runtime, Thing* func, int argNo,
        Thing** args, int* error);

#endif /* EXECUTE_H_ */
