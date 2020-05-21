#ifndef RUNTIME_H_
#define RUNTIME_H_

#include "main/util.h"

typedef void Thing;

typedef struct {
    Thing* value;
    uint8_t error;
} RetVal;

RetVal createRetVal(Thing* value, uint8_t error);
uint8_t isRetValError(RetVal val);
Thing* getRetVal(RetVal val);

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

typedef RetVal (*ExecFunc)(Runtime*, Thing*, Thing**, uint8_t);

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

#endif /* RUNTIME_H_ */
