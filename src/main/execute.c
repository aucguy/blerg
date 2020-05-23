#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main/bytecode.h"
#include "main/execute.h"
#include "main/thing.h"
#include "main/lib.h"

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

    setScopeLocal(builtins, "false", createBoolThing(runtime, 0));
    setScopeLocal(builtins, "true", createBoolThing(runtime, 1));
    setScopeLocal(builtins, "print", createNativeFuncThing(runtime, libPrint));
    setScopeLocal(builtins, "input", createNativeFuncThing(runtime, libInput));
    setScopeLocal(builtins, "assert", createNativeFuncThing(runtime, libAssert));
    setScopeLocal(builtins, "toStr", createNativeFuncThing(runtime, libToStr));
    setScopeLocal(builtins, "toInt", createNativeFuncThing(runtime, libToInt));
    setScopeLocal(builtins, "trycatch", createNativeFuncThing(runtime, libTryCatch));

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

void unwindStackFrame(Runtime* runtime, uint32_t initStackFrameSize) {
    while(stackFrameSize(runtime) > initStackFrameSize) {
        popStackFrame(runtime);
    }
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

const char* readConstantModule(Module* module, uint32_t index) {
    return module->constants[readU32Module(module, index)];
}

StackFrame* createFrameCall(Runtime* runtime, FuncThing* func, uint32_t argNo,
        Thing** args, uint8_t* error) {
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
RetVal executeCode(Runtime* runtime, StackFrame* frame) {
    //TODO have index increments in the readXModule functions
    uint32_t initStackFrameSize = stackFrameSize(runtime);

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
        } else if(opcode == OP_PUSH_BUILTIN) {
            const char* constant = readConstantModule(module, index);
            index += 4;
            Thing* value = getMapStr(runtime->operators, constant);
            if(value == NULL) {
                unwindStackFrame(runtime, initStackFrameSize);
                const char* format = "internal error: builtin '%s' not found";
                return throwMsg(runtime, formatStr(format, constant));
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
            Scope* scope = currentFrame->def.scope;
            Thing* toPush = createFuncThing(runtime, entry, module, scope);
            pushStack(runtime, toPush);
        } else if(opcode == OP_LOAD) {
            const char* constant = readConstantModule(module, index);
            index += 4;
            pushStack(runtime, getScopeValue(currentFrame->def.scope, constant));
        } else if(opcode == OP_STORE) {
            const char* constant = readConstantModule(module, index);
            index += 4;
            Thing* value = popStack(runtime);
            putMapStr(currentFrame->def.scope->locals, constant, value);
        } else if(opcode == OP_CALL) {
            //TODO fix conversion
            uint8_t arity = (unsigned int) readU32Module(module, index);
            index += 4;
            FuncThing* func = peekStackIndex(runtime, arity);
            Thing** args = malloc(arity * sizeof(Thing*));
            for(uint8_t i = 0; i < arity; i++) {
                args[arity - i - 1] = popStack(runtime);
            }
            popStack(runtime); //pop the function
            if(typeOfThing(func) == THING_TYPE_FUNC) {
                uint8_t error = 0;
                StackFrame* frame = createFrameCall(runtime, func, arity, args,
                        &error);
                if(error) {
                    unwindStackFrame(runtime, initStackFrameSize);
                    free(args);
                    //TODO make this error message better
                    const char* msg = "error creating stack frame for"
                            "function call";
                    return throwMsg(runtime, newStr(msg));
                }
                pushStackFrame(runtime, frame);
            } else {
                ThingHeader* header = customDataToThingHeader(func);
                pushStackFrame(runtime, createStackFrameNative());
                RetVal ret = header->type->call(runtime, func, args, arity);
                popStackFrame(runtime);

                if(isRetValError(ret)) {
                    unwindStackFrame(runtime, initStackFrameSize);
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

            if(typeOfThing(condition) != THING_TYPE_BOOL) {
                unwindStackFrame(runtime, initStackFrameSize);
                //TODO report what type was found
                const char* msg =" boolean needed for branches, but a boolean"
                        "was not found";
                return throwMsg(runtime, newStr(msg));
            }

            if(!thingAsBool(condition)) {
                index = target;
            }
        } else if(opcode == OP_ABS_JUMP) {
            index = readU32Module(module, index);
        } else {
            unwindStackFrame(runtime, initStackFrameSize);
            return throwMsg(runtime, "internal error: unknown bytecode");
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
    StackFrame* frame = createStackFrameDef(module, 0, scope);
    executeCode(runtime, frame);
    return createRetVal(createObjectThingFromMap(runtime, scope->locals), 0);
}

RetVal callFunction(Runtime* runtime, Thing* func, uint32_t argNo, Thing** args) {
    uint8_t error = 0;
    StackFrame* frame = createFrameCall(runtime, func, argNo, args, &error);
    if(error) {
        return throwMsg(runtime, "error creating function stack frame");
    }
    return executeCode(runtime, frame);
}
