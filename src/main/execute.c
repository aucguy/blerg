#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main/bytecode.h"
#include "main/execute.h"
#include "main/thing.h"

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

/**
 * Creates a StackFrame for the invocation of the function.
 */
StackFrame* createStackFrameDef(Module* module, uint32_t index, Scope* scope) {
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


Thing* printFunc(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error) {
    if(arity != 1 || typeOfThing(args[0]) != THING_TYPE_STR) {
        *error = 1;
        return NULL;
    }

    printf("%s\n", thingAsStr(args[0]));
    return runtime->noneThing;
}

//TODO accept arbitrary long line lengths and remove excess data
Thing* inputFunc(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error) {

    char* str = malloc(sizeof(char) * 100);
    fgets(str, 100, stdin);
    //remove newline
    str[strlen(str) - 1] = 0;
    return createStrThing(runtime, str, 0);
}

Runtime* createRuntime() {
    Runtime* runtime = malloc(sizeof(Runtime));
    runtime->stackFrame = NULL;
    runtime->stack = NULL;
    runtime->allocatedThings = NULL;
    runtime->allocatedScopes = NULL;
    runtime->noneThing = createNoneThing(runtime);

    Map* ops = createMap();
    runtime->operators = ops;

    putMapStr(ops, "+", createSymbolThing(runtime, SYM_ADD, 2));
    putMapStr(ops, "-", createSymbolThing(runtime, SYM_SUB, 2));
    putMapStr(ops, "*", createSymbolThing(runtime, SYM_MUL, 2));
    putMapStr(ops, "/", createSymbolThing(runtime, SYM_DIV, 2));
    putMapStr(ops, "==", createSymbolThing(runtime, SYM_EQ, 2));
    putMapStr(ops, "!=", createSymbolThing(runtime, SYM_NOT_EQ, 2));
    putMapStr(ops, "<", createSymbolThing(runtime, SYM_LESS_THAN, 2));
    putMapStr(ops, "<=", createSymbolThing(runtime, SYM_LESS_THAN_EQ, 2));
    putMapStr(ops, ">", createSymbolThing(runtime, SYM_GREATER_THAN, 2));
    putMapStr(ops, ">=", createSymbolThing(runtime, SYM_GREATER_THAN_EQ, 2));
    putMapStr(ops, "and", createSymbolThing(runtime, SYM_ADD, 2));
    putMapStr(ops, "or", createSymbolThing(runtime, SYM_OR, 2));
    putMapStr(ops, "not", createSymbolThing(runtime, SYM_NOT, 1));

    Scope* builtins = createScope(runtime, NULL);
    runtime->builtins = builtins;

    setScopeLocal(builtins, "print", createNativeFuncThing(runtime, printFunc));
    setScopeLocal(builtins, "input", createNativeFuncThing(runtime, inputFunc));

    return runtime;
}

void destroyRuntime(Runtime* runtime) {
    destroyList(runtime->stackFrame, free);
    destroyShallowList(runtime->stack);
    destroyList(runtime->allocatedThings, destroyThing);
    destroyList(runtime->allocatedScopes, destroyScope);
    destroyMap(runtime->operators, nothing, nothing);
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

uint32_t stackFrameSize(Runtime* runtime) {
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

Thing* peekStackIndex(Runtime* runtime, uint32_t index) {
    List* stack = runtime->stack;
    for(uint32_t i = 0; i < index; i++) {
        stack = stack->tail;
    }
    return stack->head;
}

/**
 * Reads an int operand from the bytecode. Advances the index past the operand.
 */
int32_t readI32Frame(StackFrame* frame) {
    const uint8_t* bytecode = frame->def.module->bytecode;
    uint32_t index = frame->def.index;
    int32_t arg = bytecode[index] << 24;
    arg |= bytecode[index + 1] << 16;
    arg |= bytecode[index + 2] << 8;
    arg |= bytecode[index + 3];
    frame->def.index += 4;
    return arg;
}

/**
 * Reads an unsigned int operand at the given index in the given module.
 */
uint32_t readU32Module(Module* module, uint32_t index) {
    uint32_t arg = module->bytecode[index] << 24;
    arg |= module->bytecode[index + 1] << 16;
    arg |= module->bytecode[index + 2] << 8;
    arg |= module->bytecode[index + 3];
    return arg;
}

const char* readConstantModule(Module* module, uint32_t index) {
    return module->constants[readU32Module(module, index)];
}

/**
 * Reads an unsigned int operand from the bytecode. Advances the index past the
 * operand.
 */
uint32_t readU32Frame(StackFrame* frame) {
    uint32_t arg = readU32Module(frame->def.module, frame->def.index);
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

Thing* createFrameCall(Runtime* runtime, FuncThing* func, uint32_t argNo, Thing** args,
        uint8_t* error) {
    //currently, native code can only call blerg code
    if(typeOfThing(func) != THING_TYPE_FUNC) {
        *error = 1;
        return NULL;
    }
    FuncThing* funcThing = func;
    uint32_t index = funcThing->entry;
    if(funcThing->module->bytecode[index++] != OP_DEF_FUNC) {
        *error = 1;
        return NULL;
    }

    //check that the provided number of arguments equals the function's arity
    if(funcThing->module->bytecode[index++] != argNo) {
        *error = 1;
        return NULL;
    }

    //assign the arguments provided to the names of the variables in the local
    //scope.
    Scope* scope = createScope(runtime, funcThing->parentScope);
    for(uint32_t i = 0; i < argNo; i++) {
        const char* constant = readConstantModule(funcThing->module, index);
        setScopeLocal(scope, constant, args[i]);
        index += 4;
    }

    return createStackFrameDef(func->module, index, scope);
}

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
Thing* executeCode(Runtime* runtime, StackFrame* frame, uint8_t* error) {
    uint32_t initStackFrameSize = stackFrameSize(runtime);

    pushStackFrame(runtime, frame);

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
            int32_t value = readI32Frame(currentFrame);
            pushStack(runtime, createIntThing(runtime, value));
        } else if(opcode == OP_PUSH_BUILTIN) {
            const char* constant = readConstant(currentFrame);
            Thing* value = getMapStr(runtime->operators, constant);
            if(value == NULL) {
                *error = 1;
                return NULL;
            }
            pushStack(runtime, value);
        } else if(opcode == OP_PUSH_LITERAL) {
            Thing* value = createStrThing(runtime, readConstant(currentFrame), 1);
            pushStack(runtime, value);
        } else if(opcode == OP_PUSH_NONE) {
            pushStack(runtime, runtime->noneThing);
        } else if(opcode == OP_RETURN) {
            Thing* retVal = popStack(runtime);
            popStackFrame(runtime);
            pushStack(runtime, retVal);
        } else if(opcode == OP_CREATE_FUNC) {
            uint32_t entry = readU32Frame(currentFrame);
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
            //TODO fix conversion
            uint8_t arity = (unsigned int) readU32Frame(currentFrame);
            FuncThing* func = peekStackIndex(runtime, arity);
            Thing** args = malloc(arity * sizeof(Thing*));
            for(uint8_t i = 0; i < arity; i++) {
                args[arity - i - 1] = popStack(runtime);
            }
            popStack(runtime); //pop the function
            if(typeOfThing(func) == THING_TYPE_FUNC) {
                pushStackFrame(runtime, createFrameCall(runtime, func, arity, args, error));
                if(*error) {
                    free(args);
                    return NULL;
                }
            } else {
                ThingHeader* header = customDataToThingHeader(func);
                Thing* ret = header->type->call(runtime, func, args, arity, error);
                if(*error == 1) {
                    free(args);
                    return NULL;
                }
                pushStack(runtime, ret);
            }
            free(args);
        } else if(opcode == OP_COND_JUMP_FALSE) {
            Thing* condition = popStack(runtime);
            uint32_t target = readU32Frame(currentFrame);

            if(typeOfThing(condition) != THING_TYPE_BOOL) {
                *error = 1;
                return NULL;
            }

            if(!thingAsBool(condition)) {
                currentFrame->def.index = target;
            }
        } else if(opcode == OP_ABS_JUMP) {
            currentFrame->def.index = readU32Frame(currentFrame);
        } else {
            //unknown opcode
            *error = 1;
            return NULL;
        }
    }

    return popStack(runtime);
}

Thing* executeModule(Runtime* runtime, Module* module, uint8_t* error) {
    //modules have no global scope.
    Scope* scope = createScope(runtime, runtime->builtins);
    StackFrame* frame = createStackFrameDef(module, 0, scope);
    executeCode(runtime, frame, error);
    return createObjectThingFromMap(runtime, scope->locals);
}

Thing* callFunction(Runtime* runtime, Thing* func, uint32_t argNo, Thing** args,
        uint8_t* error) {
    StackFrame* frame = createFrameCall(runtime, func, argNo, args, error);
    if(*error) {
        return NULL;
    }
    return executeCode(runtime, frame, error);
}
