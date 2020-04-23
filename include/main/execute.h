#ifndef EXECUTE_H_
#define EXECUTE_H_

#include <stdint.h>

#include "main/codegen.h"
#include "main/util.h"

/**
 * Every 'object' in the language is internally referred to as a 'thing.'
 * So there are IntThing, LiteralThing, FuncThing (results of defs) and
 * ObjectThing. Each thing is stored as a Thing struct followed by custom data
 * which is specified by its type (Int, Literal, Func, Object, etc).
 *
 * The following does not apply
 *
 * Additionally, there are currently some restrictions on which how code can
 * execute. This will later be removed.
 *
 * 1) Native code can only call blerg code. Passing a thing that does not
 * represent code defined in blerg to callFunction will result in an error.
 *
 * 2) Blerg code cannot call other blerg code. Attempting to call a function
 * that was defined in blerg will result in an error.
 */

typedef void Thing;
typedef struct Scope Scope;

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
    Map* operators;
    Scope* builtins;
} Runtime;

Runtime* createRuntime();
void destroyRuntime(Runtime* runtime);

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
Thing* executeModule(Runtime* runtime, Module* module, uint8_t* error);

/**
 * Calls the given thing with the given arguments. Currently, partial
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
Thing* callFunction(Runtime* runtime, Thing* func, uint32_t argNo,
        Thing** args, uint8_t* error);

/**
 * Each stackframe and function needs to "remember" things bound to variables.
 * This is the structure that "remembers" them.
 */
struct Scope {
    //the scope that this scope was created in. If a variable is not found
    //locally, it will be looked up in the parent scope. If null, this scope
    //has no parent scope.
    struct Scope* parent;
    //Map of const char* to Thing*. Represents variables bound in the current
    //scope.
    Map* locals;
};

#endif /* EXECUTE_H_ */
