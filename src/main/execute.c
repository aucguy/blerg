#include <stdlib.h>
#include <string.h>

#include "main/bytecode.h"
#include "main/execute.h"

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

Thing* intCall(Runtime* runtime, Thing* self, Thing** args, int* error);

void destroyObjectThing(Thing* thing);
void destroyPartialThing(Thing* thing);

static int initialized = 0;

void initExecute() {
    if(!initialized) {
        THING_TYPE_NONE = createThingType();
        setDestroyThingType(THING_TYPE_NONE, destroySimpleThing);
        setCallThingType(THING_TYPE_NONE, errorCall);
        setArityThingType(THING_TYPE_NONE, arityOne);

        THING_TYPE_SYMBOL = createThingType();
        setDestroyThingType(THING_TYPE_SYMBOL, destroySimpleThing);
        setCallThingType(THING_TYPE_SYMBOL, errorCall);
        setArityThingType(THING_TYPE_SYMBOL, arityOne);

        THING_TYPE_INT = createThingType();
        setDestroyThingType(THING_TYPE_INT, destroySimpleThing);
        setCallThingType(THING_TYPE_INT, intCall);
        setArityThingType(THING_TYPE_INT, arityTwo);

        THING_TYPE_OBJ = createThingType();
        setDestroyThingType(THING_TYPE_OBJ, destroyObjectThing);
        setCallThingType(THING_TYPE_OBJ, errorCall);
        setArityThingType(THING_TYPE_OBJ, arityOne);

        THING_TYPE_FUNC = createThingType();
        setDestroyThingType(THING_TYPE_FUNC, destroySimpleThing);
        setCallThingType(THING_TYPE_FUNC, errorCall);
        setArityThingType(THING_TYPE_FUNC, arityOne);

        THING_TYPE_PARTIAL = createThingType();
        setDestroyThingType(THING_TYPE_PARTIAL, destroyPartialThing);
        //these can be null since the interpreter handles partials specially
        //so these will never be accessed.
        setCallThingType(THING_TYPE_PARTIAL, NULL);
        setArityThingType(THING_TYPE_PARTIAL, NULL);

        initialized = 1;
    }
}

void deinitExecute() {
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

        free(THING_TYPE_PARTIAL);
        THING_TYPE_PARTIAL = NULL;

        initialized = 0;
    }
}

ThingType* typeOfThing(Thing* thing) {
    return thing->type;
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

Thing* getScopeValue(Scope* scope, const char* name) {
    Thing* value = getMapStr(scope->locals, name);
    if(value != NULL) {
        return value;
    } else if(scope->locals != NULL) {
        return getScopeValue(scope->parent, name);
    } else {
        return NULL;
    }
}

void setScopeLocal(Scope* scope, const char* name, Thing* value) {
    putMapStr(scope->locals, name, value);
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

typedef struct {
    const char* value;
} SymbolThing;

Thing* intCall(Runtime* runtime, Thing* self, Thing** args, int* error) {
    Thing* operation = args[0];
    Thing* operand = args[1];
    if(typeOfThing(operation) != THING_TYPE_SYMBOL) {
        *error = 1;
        return NULL;
    }
    SymbolThing* symbol = thingCustomData(operation);
    if(strcmp(symbol->value, "+") != 0) {
        *error = 1;
        return NULL;
    }
    if(typeOfThing(operand) != THING_TYPE_INT) {
        *error = 1;
        return NULL;
    }
    return createIntThing(runtime, thingAsInt(self) + thingAsInt(operand));
}

Thing* createSymbolThing(Runtime* runtime, const char* value) {
    Thing* thing = createThing(runtime, THING_TYPE_SYMBOL, sizeof(SymbolThing));
    SymbolThing* symbolThing = thingCustomData(thing);
    symbolThing->value = value;
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
 * PartialThing represents a partial application of a function. If a function's
 * arguments are not fully applied during a call operation, its arguments are
 * stored in a PartialThing.
 */
typedef struct {
    //the function to call once all the arguments are provided
    Thing* func;
    //the number of arguments applied in this partial application
    unsigned char provided;
    //an array of arguments applied in this partial application. The length is
    //equal to provided.
    Thing** applied;
} PartialThing;

Thing* createPartialThing(Runtime* runtime, Thing* func, unsigned char provided, Thing** applied) {
    Thing* thing = createThing(runtime, THING_TYPE_PARTIAL, sizeof(PartialThing));
    PartialThing* partialThing = thingCustomData(thing);
    partialThing->func = func;
    partialThing->provided = provided;
    partialThing->applied = applied;
    return thing;
}

void destroyPartialThing(Thing* thing) {
    PartialThing* partialThing = thingCustomData(thing);
    free(partialThing->applied);
    free(thing);
}

/**
 * Creates an array of things whose length is 1 and whose sole element is the
 * given argument
 */
Thing** createSingletonThingArray(Thing* thing) {
    Thing** array = malloc(sizeof(Thing*));
    array[0] = thing;
    return array;
}

/**
 * Copies the array, extends the new array by one element and sets that element
 * to add.
 *
 * @param length the number of elements in original
 * @param original the elements that should be the start of the returned array
 * @param add the last element of the returned array
 * @return an array whose length is length + 1, whose ith element is
 *      original[i] for 0 <= i < length and original[length] = add.
 */
Thing** appendThingArray(unsigned int length, Thing** original, Thing* add) {
    Thing** modified = malloc(sizeof(Thing*) * (length + 1));
    for(unsigned int i = 0; i < length; i++) {
        modified[i] = original[i];
    }
    modified[length] = add;
    return modified;
}

typedef struct {
    //the module that is currently executing.
    Module* module;
    //the current bytecode index
    unsigned int index;
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

/**
 * Creates a StackFrame for the invocation of the function.
 */
StackFrame* createStackFrameDef(Module* module, unsigned int index, Scope* scope) {
    StackFrame* frame = malloc(sizeof(StackFrame));
    frame->type = STACK_FRAME_DEF;
    frame->def.module = module;
    frame->def.index = index;
    frame->def.scope = scope;
    return frame;
}

StackFrame* createStackFrameNative() {
    StackFrame* frame = malloc(sizeof(StackFrame));
    frame->type = STACK_FRAME_NATIVE;
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
    const unsigned char* bytecode = frame->def.module->bytecode;
    unsigned char index = frame->def.index;
    int arg = bytecode[index] << 24;
    arg |= bytecode[index + 1] << 16;
    arg |= bytecode[index + 2] << 8;
    arg |= bytecode[index + 3];
    frame->def.index += 4;
    return arg;
}

/**
 * Reads an unsigned int operand at the given index in the given module.
 */
unsigned int readU32Module(Module* module, unsigned int index) {
    unsigned int arg = module->bytecode[index] << 24;
    arg |= module->bytecode[index + 1] << 16;
    arg |= module->bytecode[index + 2] << 8;
    arg |= module->bytecode[index + 3];
    return arg;
}

const char* readConstantModule(Module* module, unsigned int index) {
    return module->constants[readU32Module(module, index)];
}

/**
 * Reads an unsigned int operand from the bytecode. Advances the index past the
 * operand.
 */
unsigned int readU32Frame(StackFrame* frame) {
    unsigned int arg = readU32Module(frame->def.module, frame->def.index);
    frame->def.index += 4;
    return arg;
}

/**
 * Reads a constant operand (really just an index to the pool). Advances the
 * index past the operand.
 */
const char* readConstant(StackFrame* frame) {
    return frame->def.module->constants[readU32Frame(frame)];
}

typedef struct {
    Runtime* runtime;
    Module* entryModule;
    unsigned int entryIndex;
    Scope* bottomScope;
} ExecCodeArgs;

/**
 * Executes the given bytecode. Since there are so many arguments, the
 * arguments are stored in a struct.
 *
 * @param allArgs.runtime the runtime object
 * @param allArgs.entryModule the module of the code being executed
 * @param allArgs.entryIndex the index to start execution
 * @param allArgs.bottomScope the scope that the code is executed under. It is
 *          the scope of the bottom stackframe.
 * @param error set to a nonzero value upon error
 * @returns the value returned from the invocation / bottom stackframe.
 */
Thing* executeCode(ExecCodeArgs allArgs, int* error) {
    Runtime* runtime = allArgs.runtime;

    int initStackFrameSize = stackFrameSize(runtime);

    pushStackFrame(runtime, createStackFrameDef(allArgs.entryModule,
            allArgs.entryIndex, allArgs.bottomScope));

    while(stackFrameSize(runtime) > initStackFrameSize) {
        StackFrame* currentFrame = currentStackFrame(runtime);
        //sanity check, native frames should end before the interpreter loop.
        //if this fails then there is some sort of internal error.
        if(currentFrame->type != STACK_FRAME_DEF) {
            *error = 1;
            return NULL;
        }
        Module* module = currentFrame->def.module;
        unsigned char opcode = module->bytecode[currentFrame->def.index++];

        if(opcode == OP_PUSH_INT) {
            int value = readI32Frame(currentFrame);
            pushStack(runtime, createIntThing(runtime, value));
        } else if(opcode == OP_PUSH_SYMBOL) {
            const char* constant = readConstant(currentFrame);
            pushStack(runtime, createSymbolThing(runtime, constant));
        } else if(opcode == OP_PUSH_NONE) {
            pushStack(runtime, runtime->noneThing);
        } else if(opcode == OP_RETURN) {
            Thing* retVal = popStack(runtime);
            popStackFrame(runtime);
            pushStack(runtime, retVal);
        } else if(opcode == OP_CREATE_FUNC) {
            int entry = readU32Frame(currentFrame);
            Scope* scope = currentFrame->def.scope;
            Thing* toPush = createFuncThing(runtime, entry, module, scope);
            pushStack(runtime, toPush);
        } else if(opcode == OP_LOAD) {
            const char* constant = readConstant(currentFrame);
            pushStack(runtime, getScopeValue(currentFrame->def.scope, constant));
        } else if(opcode == OP_STORE) {
            const char* constant = readConstant(currentFrame);
            Thing* value = popStack(runtime);
            putMapStr(currentFrame->def.scope->locals, constant, value);
        } else if(opcode == OP_CALL) {
            Thing* arg = popStack(runtime);
            Thing* called = popStack(runtime);

            Thing* func = NULL;
            unsigned char provided = 0;
            Thing** applied = NULL;
            if(typeOfThing(called) == THING_TYPE_PARTIAL) {
                PartialThing* partial = thingCustomData(called);
                provided = partial->provided + 1;
                applied = appendThingArray(partial->provided, partial->applied,
                        arg);
                func = partial->func;
            } else {
                provided = 1;
                applied = createSingletonThingArray(arg);
                func = called;
            }

            if(provided == func->type->arity(func)) {
                if(typeOfThing(func) == THING_TYPE_FUNC) {
                    //calling functions defined in blerg from blerg code is not
                    //yet supported.
                    *error = 1;
                    return NULL;
                } else {
                    pushStackFrame(runtime, createStackFrameNative());
                    Thing* retVal = func->type->call(runtime, func, applied, error);
                    if(*error) {
                        return NULL;
                    }
                    pushStack(runtime, retVal);
                    popStackFrame(runtime);
                }
                free(applied);
            } else {
                Thing* partial = createPartialThing(runtime, func,
                        provided, applied);
                pushStack(runtime, partial);
            }
        } else {
            //unknown opcode
            *error = 1;
            return NULL;
        }
    }

    return popStack(runtime);
}

Thing* executeModule(Runtime* runtime, Module* module, int* error) {
    ExecCodeArgs allArgs;
    allArgs.runtime = runtime;
    allArgs.entryModule = module;
    allArgs.entryIndex = 0;
    //modules have no global scope.
    allArgs.bottomScope = createScope(runtime, NULL);
    executeCode(allArgs, error);
    return createObjectThingFromMap(runtime, allArgs.bottomScope->locals);
}

Thing* callFunction(Runtime* runtime, Thing* func, int argNo, Thing** args,
        int* error) {
    //currently, native code can only call blerg code
    if(typeOfThing(func) != THING_TYPE_FUNC) {
        *error = 1;
        return NULL;
    }
    FuncThing* funcThing = thingCustomData(func);
    int index = funcThing->entry;
    if(funcThing->module->bytecode[index++] != OP_DEF_FUNC) {
        *error = 1;
        return NULL;
    }
    //currently partial application native code is not supported, so the number
    //of arguments provided must match the arity of the function
    if(funcThing->module->bytecode[index++] != argNo) {
        *error = 1;
        return NULL;
    }
    //assign the arguments provided to the names of the variables in the local
    //scope.
    Scope* scope = createScope(runtime, funcThing->parentScope);
    for(int i = 0; i < argNo; i++) {
        const char* constant = readConstantModule(funcThing->module, index);
        setScopeLocal(scope, constant, args[i]);
        index += 4;
    }
    ExecCodeArgs allArgs;
    allArgs.runtime = runtime;
    allArgs.entryModule = funcThing->module;
    allArgs.entryIndex = index;
    allArgs.bottomScope = scope;
    return executeCode(allArgs, error);
}
