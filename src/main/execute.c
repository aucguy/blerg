#include <stdlib.h>

#include "main/bytecode.h"
#include "main/execute.h"

ThingType* createThingType() {
    ThingType* type = malloc(sizeof(ThingType));
    type->destroy = NULL;
    return type;
}

void setDestroyThingType(ThingType* type, void(*destroy)(Thing*)) {
    type->destroy = destroy;
}

ThingType* typeOfThing(Thing* thing) {
    return thing->type;
}

void destroySimpleThing(Thing* thing) {
    free(thing);
}

void destroyObjectThing(Thing* thing);

static int initialized = 0;

void initExecute() {
    if(!initialized) {
        THING_TYPE_NONE = createThingType();
        setDestroyThingType(THING_TYPE_NONE, destroySimpleThing);

        THING_TYPE_INT = createThingType();
        setDestroyThingType(THING_TYPE_INT, destroySimpleThing);

        THING_TYPE_OBJ = createThingType();
        setDestroyThingType(THING_TYPE_OBJ, destroyObjectThing);

        THING_TYPE_FUNC = createThingType();
        setDestroyThingType(THING_TYPE_FUNC, destroySimpleThing);

        initialized = 1;
    }
}

void deinitExecute() {
    if(initialized) {
        free(THING_TYPE_NONE);
        THING_TYPE_NONE = NULL;

        free(THING_TYPE_INT);
        THING_TYPE_INT = NULL;

        free(THING_TYPE_OBJ);
        THING_TYPE_OBJ = NULL;

        free(THING_TYPE_FUNC);
        THING_TYPE_FUNC = NULL;

        initialized = 0;
    }
}

/**
 * Each stackframe and function needs to "remember" things bound to variables.
 * This is the structure that "remembers" them.
 */
typedef struct Scope_ {
    //the scope that this scope was created in. If a variable is not found
    //locally, it will be looked up in the parent scope. If null, this scope
    //has no parent scope.
    struct Scope_* parent;
    //Map of const char* to Thing*. Represents variables bound in the current
    //scope.
    Map* locals;
} Scope;

Scope* createScope(Runtime* runtime, Scope* parent) {
    Scope* scope = malloc(sizeof(Scope));
    scope->parent = parent;
    scope->locals = createMap();
    runtime->allocatedScopes = consList(scope, runtime->allocatedScopes);
    return scope;
}

void destroyScope(void* scope) {
    destroyMap(((Scope*) scope)->locals, nothing, nothing);
    free(scope);
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

/**
 * Represents functions whose instructions are defined in blerg code.
 * I.E. 'f'in 'def f ...', but not '1'. This is also used to represent the code
 * used to initialize the module object.
 */
typedef struct {
    //location of first bytecode of the function
    unsigned int entry;
    //module the function was declared in
    Module* module;
    //the scope the function was declared in
    Scope* parentScope;
} FuncThing;

Thing* createFuncThing(Runtime* runtime, unsigned int entry,
        Module* module, Scope* parentScope) {
    Thing* thing = createThing(runtime, THING_TYPE_FUNC, sizeof(FuncThing));
    FuncThing* funcThing = thingCustomData(thing);
    funcThing->entry = entry;
    funcThing->module = module;
    funcThing->parentScope = parentScope;
    return thing;
}

/**
 * Holds one item of the call stack. This keeps track of the current execution
 * state for a given invocation. Multiple of these can exist as functions call
 * other functions.
 */
typedef struct {
    //the module that is currently executing.
    Module* module;
    //the current bytecode index
    unsigned int index;
    //the current scope
    Scope* scope;
} StackFrame;

/**
 * Creates a StackFrame for the invocation of the function.
 */
StackFrame* createFuncStackFrame(Runtime* runtime, FuncThing* func) {
    StackFrame* frame = malloc(sizeof(StackFrame));
    frame->module = func->module;
    frame->index = func->entry;
    frame->scope = createScope(runtime, func->parentScope);
    return frame;
}

Runtime* createRuntime() {
    Runtime* runtime = malloc(sizeof(Runtime));
    runtime->stackFrame = NULL;
    runtime->stack = NULL;
    runtime->allocatedThings = NULL;
    runtime->allocatedScopes = NULL;
    runtime->noneThing = createNoneThing(runtime);
    return runtime;
}

void callDestroyMethod(void* thing) {
    ((Thing*) thing)->type->destroy((Thing*) thing);
}

void destroyRuntime(Runtime* runtime) {
    destroyList(runtime->stackFrame, free);
    destroyShallowList(runtime->stack);
    destroyList(runtime->allocatedThings, callDestroyMethod);
    destroyList(runtime->allocatedScopes, destroyScope);
    free(runtime);
}

StackFrame* currentStackFrame(Runtime* runtime) {
    return runtime->stackFrame->head;
}

void pushStackFrame(Runtime* runtime, StackFrame* frame) {
    runtime->stackFrame = consList(frame, runtime->stackFrame);
}

void popStackFrame(Runtime* runtime) {
    List* toDelete = runtime->stackFrame;
    runtime->stackFrame = runtime->stackFrame->tail;
    free(toDelete->head);
    free(toDelete);
}

int stackFrameSize(Runtime* runtime) {
    return lengthList(runtime->stackFrame);
}

void pushStack(Runtime* runtime, Thing* thing) {
    runtime->stack = consList(thing, runtime->stack);
}

Thing* popStack(Runtime* runtime) {
    Thing* thing = runtime->stack->head;
    List* toDelete = runtime->stack;
    runtime->stack = runtime->stack->tail;
    free(toDelete);
    return thing;
}

/**
 * Reads an int operand from the bytecode. Advances the index past the operand.
 */
int readI32Frame(StackFrame* frame) {
    int arg = frame->module->bytecode[frame->index] << 24;
    arg |= frame->module->bytecode[frame->index + 1] << 16;
    arg |= frame->module->bytecode[frame->index + 2] << 8;
    arg |= frame->module->bytecode[frame->index + 3];
    frame->index += 4;
    return arg;
}

/**
 * Reads an unsigned int operand from the bytecode. Advances the index past the
 * operand.
 */
unsigned int readU32Frame(StackFrame* frame) {
    unsigned int arg = frame->module->bytecode[frame->index] << 24;
    arg |= frame->module->bytecode[frame->index + 1] << 16;
    arg |= frame->module->bytecode[frame->index + 2] << 8;
    arg |= frame->module->bytecode[frame->index + 3];
    frame->index += 4;
    return arg;
}

/**
 * Reads a constant operand (really just an index to the pool). Advances the
 * index past the operand.
 */
const char* readConstant(StackFrame* frame) {
    return frame->module->constants[readU32Frame(frame)];
}

/**
 * Executes the given given function. This function is used internally. Instead
 * use executeModule or executeFunction to execute bytecode.
 *
 * @param runtime the runtime object
 * @param func the function to execute
 * @param error set to a nonzero value upon error
 * @param flag if 1, the local scope of the function is returned, otherwise
 *          whatever the function returned is returned.
 */
Thing* executeFuncSpecial(Runtime* runtime, Thing* func, int* error, int flag) {
    if(typeOfThing(func) != THING_TYPE_FUNC) {
        *error = 1;
        return NULL;
    }

    FuncThing* funcThing = thingCustomData(func);
    int initStackFrameSize = stackFrameSize(runtime);
    pushStackFrame(runtime, createFuncStackFrame(runtime, funcThing));
    Scope* global = currentStackFrame(runtime)->scope;

    while(stackFrameSize(runtime) > initStackFrameSize) {
        StackFrame* currentFrame = currentStackFrame(runtime);
        Module* module = currentFrame->module;
        unsigned char opcode = module->bytecode[currentFrame->index++];

        if(opcode == OP_PUSH_INT) {
            int value = readI32Frame(currentFrame);
            pushStack(runtime, createIntThing(runtime, value));
        } else if(opcode == OP_PUSH_NONE) {
            pushStack(runtime, runtime->noneThing);
        } else if(opcode == OP_RETURN) {
            Thing* retVal = popStack(runtime);
            popStackFrame(runtime);
            pushStack(runtime, retVal);
        } else if(opcode == OP_CREATE_FUNC) {
            int entry = readU32Frame(currentFrame);
            Scope* scope = currentFrame->scope;
            Thing* toPush = createFuncThing(runtime, entry, module, scope);
            pushStack(runtime, toPush);
        } else if(opcode == OP_STORE) {
            const char* constant = readConstant(currentFrame);
            Thing* value = popStack(runtime);
            putMapStr(currentFrame->scope->locals, constant, value);
        }
    }

    if(flag) {
        return createObjectThingFromMap(runtime, global->locals);
    } else {
        return popStack(runtime);
    }
}

Thing* executeModule(Runtime* runtime, Module* module, int* error) {
    //wrap the module in a function
    Thing* entry = createFuncThing(runtime, 0, module, NULL);
    return executeFuncSpecial(runtime, entry, error, 1);
}
