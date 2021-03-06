#ifndef RUNTIME_HPP_
#define RUNTIME_HPP_

#include "main/util.h"
#include "main/bytecode.h"

typedef class Thing Thing;

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
    Map* modules;
    List* moduleBytecode;
    const char* execDir;
} Runtime;

typedef RetVal (*ExecFunc)(Runtime*, Thing*, Thing**, uint8_t);

typedef uint32_t ThingType;

extern ThingType TYPE_NONE;
extern ThingType TYPE_INT;
extern ThingType TYPE_FLOAT;
extern ThingType TYPE_STR;
extern ThingType TYPE_BOOL;
extern ThingType TYPE_MODULE;
extern ThingType TYPE_FUNC;
extern ThingType TYPE_NATIVE_FUNC;
extern ThingType TYPE_ERROR;
extern ThingType TYPE_TUPLE;
extern ThingType TYPE_LIST;
extern ThingType TYPE_OBJECT;
extern ThingType TYPE_CELL;
extern ThingType TYPE_SYMBOL;
extern ThingType TYPE_VARARG;
extern ThingType TYPE_UNDEF;

class Thing {
public:
    virtual ~Thing() = 0;
    virtual RetVal call(Runtime*, Thing*, Thing**, uint8_t) = 0;
    virtual RetVal dispatch(Runtime*, Thing*, Thing**, uint8_t) = 0;
    virtual ThingType type() = 0;
};

RetVal throwMsg(Runtime* runtime, const char* msg);
Thing* createErrorThing(Runtime* runtime, const char* msg);

typedef struct {
    //the module that is currently executing.
    Module* module;
    //the current bytecode index
    uint32_t index;
    //the current scope
    Scope* scope;
} StackFrameDef;

typedef struct {
    char dummy;
} StackFrameNative;

typedef enum {
    STACK_FRAME_DEF,
    STACK_FRAME_NATIVE
} STACK_FRAME_TYPE;

/**
 * Holds one item of the call stack. This keeps track of the current execution
 * state for a given invocation. Multiple of these can exist as functions call
 * other functions.
 *
 * There are two different types of stack frames; one for blerg code and one
 * for native code. The blerg code type keeps track of the execution state,
 * while the native code type simply denotes that native code is being executed.
 */
typedef struct {
    STACK_FRAME_TYPE type;
    union {
        StackFrameDef def;
        StackFrameNative native;
    };
} StackFrame;

#endif /* RUNTIME_HPP_ */
