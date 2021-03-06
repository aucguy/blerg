#include <main/thing.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main/bytecode.h"
#include "main/execute.h"
#include "main/lib.h"
#include "main/std_lib/modules.h"

#define UNUSED(x) (void)(x)

Scope* createScope(Runtime* runtime, Scope* parent) {
    Scope* scope = (Scope*) malloc(sizeof(Scope));
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
    Thing* value = (Thing*) getMapStr(scope->locals, name);
    if(value != NULL) {
        return value;
    } else if(scope->parent != NULL) {
        return getScopeValue(scope->parent, name);
    } else {
        return NULL;
    }
}

void setScopeLocal(Scope* scope, const char* name, Thing* value) {
    putMapStr(scope->locals, name, value);
}

Scope* copyScope(Runtime* runtime, Scope* oldScope) {
    Scope* newScope = createScope(runtime, oldScope);

    while(oldScope != NULL) {
        Entry* locals = oldScope->locals->entry;

        while(locals != NULL) {
            if(getMapStr(newScope->locals, (const char*) locals->key) == NULL) {
                setScopeLocal(newScope, (const char*) locals->key, (Thing*) locals->value);
            }
            locals = locals->tail;
        }

        oldScope = oldScope->parent;
    }

    return newScope;
}

/**
 * Creates a StackFrame for the invocation of the function.
 */
StackFrame* createStackFrameDef(Module* module, uint32_t index, Scope* scope) {
    StackFrame* frame = (StackFrame*) malloc(sizeof(StackFrame));
    frame->type = STACK_FRAME_DEF;
    frame->def.module = module;
    frame->def.index = index;
    frame->def.scope = scope;
    return frame;
}

StackFrame* createStackFrameNative() {
    StackFrame* frame = (StackFrame*) malloc(sizeof(StackFrame));
    frame->type = STACK_FRAME_NATIVE;
    return frame;
}

Runtime* createRuntime(uint8_t argc, const char* args[]) {
    UNUSED(argc);

    Runtime* runtime = (Runtime*) malloc(sizeof(Runtime));
    runtime->stackFrame = NULL;
    runtime->stack = NULL;
    runtime->allocatedThings = NULL;
    runtime->allocatedScopes = NULL;
    runtime->noneThing = createNoneThing(runtime);
    runtime->modules = createMap();
    runtime->moduleBytecode = NULL;

    uint32_t i;
    for(i = strlen(args[0]); i > 0; i--) {
        if(args[0][i] == '/' || args[0][i] == '\\') {
            break;
        }
    }

    runtime->execDir = sliceStr(args[0], 0, i);

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
    putMapStr(ops, "and", createSymbolThing(runtime, SYM_AND, 2));
    putMapStr(ops, "or", createSymbolThing(runtime, SYM_OR, 2));
    putMapStr(ops, "not", createSymbolThing(runtime, SYM_NOT, 1));
    putMapStr(ops, "tuple", createNativeFuncThing(runtime, libTuple));
    putMapStr(ops, "::", createNativeFuncThing(runtime, libCons));
    putMapStr(ops, "object", createNativeFuncThing(runtime, libObject));
    putMapStr(ops, "get", createSymbolThing(runtime, SYM_GET, 2));
    putMapStr(ops, "unpack_cons", createNativeFuncThing(runtime, libUnpackCons));
    putMapStr(ops, "none", runtime->noneThing);
    putMapStr(ops, ".", createSymbolThing(runtime, SYM_DOT, 2));
    putMapStr(ops, "unpack_call", createNativeFuncThing(runtime, libUnpackCall));
    putMapStr(ops, "assert_equal", createNativeFuncThing(runtime,
            libAssertEqual));

    Scope* builtins = createScope(runtime, NULL);
    runtime->builtins = builtins;

    setScopeLocal(builtins, "none", runtime->noneThing);
    setScopeLocal(builtins, "false", createBoolThing(runtime, 0));
    setScopeLocal(builtins, "true", createBoolThing(runtime, 1));
    setScopeLocal(builtins, "print", createNativeFuncThing(runtime, libPrint));
    setScopeLocal(builtins, "input", createNativeFuncThing(runtime, libInput));
    setScopeLocal(builtins, "assert", createNativeFuncThing(runtime, libAssert));
    setScopeLocal(builtins, "toStr", createNativeFuncThing(runtime, libToStr));
    setScopeLocal(builtins, "toInt", createNativeFuncThing(runtime, libToInt));
    setScopeLocal(builtins, "trycatch", createNativeFuncThing(runtime, libTryCatch));
    setScopeLocal(builtins, "head", createNativeFuncThing(runtime, libHead));
    setScopeLocal(builtins, "tail", createNativeFuncThing(runtime, libTail));
    setScopeLocal(builtins, "get", (Thing*) getMapStr(ops, "get"));
    setScopeLocal(builtins, "createSymbol",
            createNativeFuncThing(runtime, libCreateSymbol));
    setScopeLocal(builtins, "createCell",
            createNativeFuncThing(runtime, libCreateCell));
    setScopeLocal(builtins, "getCell",
            createNativeFuncThing(runtime, libGetCell));
    setScopeLocal(builtins, "setCell",
            createNativeFuncThing(runtime, libSetCell));
    setScopeLocal(builtins, "import", createNativeFuncThing(runtime, libImport));
    setScopeLocal(builtins, "responds_to", createSymbolThing(runtime, SYM_RESPONDS_TO, 2));
    setScopeLocal(builtins, "is_none", createNativeFuncThing(runtime, libIsNone));

    return runtime;
}

void destroyModuleVoid(void* module) {
    destroyModule((Module*) module);
}

void destroyRuntime(Runtime* runtime) {
    destroyList(runtime->stackFrame, free);
    destroyShallowList(runtime->stack);
    destroyList(runtime->allocatedThings, (void (*)(void*)) destroyThing);
    destroyList(runtime->allocatedScopes, destroyScope);
    destroyMap(runtime->operators, nothing, nothing);
    destroyMap(runtime->modules, nothing, nothing);
    destroyList(runtime->moduleBytecode, destroyModuleVoid);
    free((char*) runtime->execDir);
    free(runtime);
    destroyBuiltinModules();
}

StackFrame* currentStackFrame(Runtime* runtime) {
    return (StackFrame*) runtime->stackFrame->head;
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

uint32_t stackSize(Runtime* runtime) {
    return lengthList(runtime->stack);
}

Thing* popStack(Runtime* runtime);

void unwindStackFrame(Runtime* runtime, uint32_t initStackFrameSize,
        uint32_t initStackSize) {
    while(stackFrameSize(runtime) > initStackFrameSize) {
        popStackFrame(runtime);
    }

    while(stackSize(runtime) > initStackSize) {
        popStack(runtime);
    }
}

void pushStack(Runtime* runtime, Thing* thing) {
    runtime->stack = consList(thing, runtime->stack);
}

Thing* popStack(Runtime* runtime) {
    Thing* thing = (Thing*) runtime->stack->head;
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
    return (Thing*) stack->head;
}

/**
 * Reads an unsigned int operand at the given index in the given module.
 */
int32_t readI32Module(Module* module, uint32_t index) {
    int32_t arg = module->bytecode[index] << 24;
    arg |= module->bytecode[index + 1] << 16;
    arg |= module->bytecode[index + 2] << 8;
    arg |= module->bytecode[index + 3];
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

float readFloatModule(Module* module, uint32_t index) {
    //bitwise cast uint32_t to float
    uint32_t arg = readU32Module(module, index);
    return *((float*) &arg);
}

const char* readConstantModule(Module* module, uint32_t index) {
    return module->constants[readU32Module(module, index)];
}

StackFrame* createFrameCall(Runtime* runtime, Thing* func, uint32_t argNo,
        Thing** args, uint8_t* error) {
    //currently, native code can only call blerg code
    if(typeOfThing(func) != TYPE_FUNC) {
        *error = 1;
        return NULL;
    }
    uint32_t index = getFuncEntry(func);
    uint8_t opcode = getFuncModule(func)->bytecode[index++];
    if(opcode != OP_DEF_FUNC) {
        *error = 1;
        return NULL;
    }

    //check that the provided number of arguments equals the function's arity
    if(getFuncModule(func)->bytecode[index++] != argNo) {
        *error = 1;
        return NULL;
    }

    //if its the init function, the local scope is the parent scope
    Scope* scope = createScope(runtime, getFuncParentScope(func));

    //assign the arguments provided to the names of the variables in the local
    //scope.
    for(uint32_t i = 0; i < argNo; i++) {
        const char* constant = readConstantModule(getFuncModule(func), index);
        setScopeLocal(scope, constant, args[i]);
        index += 4;
    }

    return createStackFrameDef(getFuncModule(func), index, scope);
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
RetVal executeCode(Runtime* runtime, StackFrame* frame) {
    //TODO have index increments in the readXModule functions
    //TODO check if stack is empty
    uint32_t initStackFrameSize = stackFrameSize(runtime);
    uint32_t initStackSize = stackSize(runtime);

    pushStackFrame(runtime, frame);

    while(stackFrameSize(runtime) > initStackFrameSize) {
        StackFrame* currentFrame = currentStackFrame(runtime);

        //sanity check, native frames should end before the interpreter loop.
        //if this fails then there is some sort of internal error.
        if(currentFrame->type != STACK_FRAME_DEF) {
            //unwindStackFrame is not called here since the stack is messed up
            //anyway
            const char* msg = "internal error: native frame not ended before def frame";
            return throwMsg(runtime, newStr(msg));
        }

        Module* module = currentFrame->def.module;
        uint32_t index = currentFrame->def.index;
        unsigned char opcode = module->bytecode[index];
        index++;
        uint8_t storeIndex = 1;

        if(opcode == OP_PUSH_INT) {
            int32_t value = readI32Module(module, index);
            index += 4;
            pushStack(runtime, createIntThing(runtime, value));
        } else if(opcode == OP_PUSH_FLOAT) {
            float value = readFloatModule(module, index);
            index += 4;
            pushStack(runtime, createFloatThing(runtime, value));
        } else if(opcode == OP_PUSH_BUILTIN) {
            const char* constant = readConstantModule(module, index);
            index += 4;
            Thing* value = (Thing*) getMapStr(runtime->operators, constant);
            if(value == NULL) {
                const char* format = "internal error: builtin '%s' not found";
                RetVal error = throwMsg(runtime, formatStr(format, constant));
                unwindStackFrame(runtime, initStackFrameSize, initStackSize);
                return error;
            }
            pushStack(runtime, value);
        } else if(opcode == OP_PUSH_LITERAL) {
            const char* constant = readConstantModule(module, index);
            index += 4;
            Thing* value = createStrThing(runtime, constant, 1);
            pushStack(runtime, value);
        } else if(opcode == OP_PUSH_NONE) {
            pushStack(runtime, runtime->noneThing);
        } else if(opcode == OP_RETURN) {
            Thing* retVal = popStack(runtime);
            popStackFrame(runtime);
            pushStack(runtime, retVal);
            storeIndex = 0;
        } else if(opcode == OP_CREATE_FUNC) {
            uint32_t entry = readU32Module(module, index);
            index += 4;
            Scope* scope = copyScope(runtime, currentFrame->def.scope);
            Thing* toPush = createFuncThing(runtime, entry, module, scope);
            pushStack(runtime, toPush);
        } else if(opcode == OP_LOAD) {
            const char* constant = readConstantModule(module, index);
            index += 4;
            Thing* value = getScopeValue(currentFrame->def.scope, constant);
            if(value == NULL) {
                const char* msg = formatStr("'%s' is undefined", constant);
                RetVal error = throwMsg(runtime, msg);
                unwindStackFrame(runtime, initStackFrameSize, initStackSize);
                return error;
            }
            pushStack(runtime, value);
        } else if(opcode == OP_STORE) {
            const char* constant = readConstantModule(module, index);
            index += 4;
            Thing* value = popStack(runtime);
            putMapStr(currentFrame->def.scope->locals, constant, value);
        } else if(opcode == OP_CALL) {
            //TODO fix conversion
            uint8_t arity = (unsigned int) readU32Module(module, index);
            index += 4;
            Thing* func = peekStackIndex(runtime, arity);
            Thing** args = (Thing**) malloc(arity * sizeof(Thing*));
            for(uint8_t i = 0; i < arity; i++) {
                args[arity - i - 1] = popStack(runtime);
            }
            popStack(runtime); //pop the function
            if(typeOfThing(func) == TYPE_FUNC) {
                uint8_t error = 0;
                StackFrame* frame = createFrameCall(runtime, func, arity, args,
                        &error);
                if(error) {
                    free(args);
                    //TODO make this error message better
                    const char* msg = "error creating stack frame for "
                            "function call";
                    RetVal error = throwMsg(runtime, newStr(msg));
                    unwindStackFrame(runtime, initStackFrameSize, initStackSize);
                    return error;
                }
                pushStackFrame(runtime, frame);
            } else {
                pushStackFrame(runtime, createStackFrameNative());
                RetVal ret = func->call(runtime, func, args, arity);
                popStackFrame(runtime);

                if(isRetValError(ret)) {
                    unwindStackFrame(runtime, initStackFrameSize, initStackSize);
                    free(args);
                    return ret;
                }
                pushStack(runtime, getRetVal(ret));
            }
            free(args);
        } else if(opcode == OP_COND_JUMP_FALSE) {
            Thing* condition = popStack(runtime);
            uint32_t target = readU32Module(module, index);
            index += 4;

            if(typeOfThing(condition) != TYPE_BOOL) {
                //TODO report what type was found
                const char* msg =" boolean needed for branches, but a boolean "
                        "was not found";
                RetVal error = throwMsg(runtime, newStr(msg));
                unwindStackFrame(runtime, initStackFrameSize, initStackSize);
                return error;
            }

            if(!thingAsBool(condition)) {
                index = target;
            }
        } else if(opcode == OP_ABS_JUMP) {
            index = readU32Module(module, index);
        } else if(opcode == OP_DUP) {
            pushStack(runtime, peekStackIndex(runtime, 0));
        } else if(opcode == OP_ROT3) {
            Thing* value1 = popStack(runtime);
            Thing* value2 = popStack(runtime);
            Thing* value3 = popStack(runtime);
            pushStack(runtime, value2);
            pushStack(runtime, value3);
            pushStack(runtime, value1);
        } else if(opcode == OP_SWAP) {
            Thing* value1 = popStack(runtime);
            Thing* value2 = popStack(runtime);
            pushStack(runtime, value1);
            pushStack(runtime, value2);
        } else if(opcode == OP_POP) {
            popStack(runtime);
        } else if(opcode == OP_CHECK_NONE) {
            Thing* value = popStack(runtime);
            if(value != runtime->noneThing) {
                RetVal error = throwMsg(runtime, newStr("value is not none"));
                unwindStackFrame(runtime, initStackFrameSize, initStackSize);
                return error;
            }
        } else {
            const char* msg = "internal error: unknown bytecode";
            RetVal error = throwMsg(runtime, newStr(msg));
            unwindStackFrame(runtime, initStackFrameSize, initStackSize);
            return error;
        }

        if(storeIndex) {
            currentFrame->def.index = index;
        }
    }

    return createRetVal(popStack(runtime), 0);
}

RetVal executeModule(Runtime* runtime, Module* module) {
    //modules have no global scope.
    Scope* scope = createScope(runtime, runtime->builtins);
    uint32_t start = module->entryIndex;
    StackFrame* frame = createStackFrameDef(module, start, scope);
    RetVal ret = executeCode(runtime, frame);
    if(isRetValError(ret)) {
        return ret;
    } else {
        return createRetVal(createModuleThing(runtime, scope->locals), 0);
    }
}

RetVal callFunction(Runtime* runtime, Thing* func, uint32_t argNo, Thing** args) {
    if(typeOfThing(func) == TYPE_FUNC) {
        uint8_t error = 0;
        StackFrame* frame = createFrameCall(runtime, func, argNo, args, &error);
        if(error) {
            return throwMsg(runtime, "error creating function stack frame");
        }
        return executeCode(runtime, frame);
    } else {
        pushStackFrame(runtime, createStackFrameNative());
        RetVal ret = func->call(runtime, func, args, argNo);
        popStackFrame(runtime);
        return ret;
    }
}
