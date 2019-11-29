#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main/parse.h"
#include "main/validate.h"
#include "main/transform.h"
#include "main/codegen.h"

#include "test/tests.h"

int readInt(Module* module, int* index) {
    int arg = module->bytecode[*index] << 24;
    arg |= module->bytecode[*index + 1] << 16;
    arg |= module->bytecode[*index + 2] << 8;
    arg |= module->bytecode[*index + 3];
    *index += 4;
    return arg;
}

const char* getConstant(Module* module, int arg) {
    if(arg < module->constantsLength) {
        return module->constants[arg];
    } else {
        return "!corrupt bytecode!";
    }
}

void printIndexArgOp(const char* opName, Module* module, int* index) {
    int arg = readInt(module, index);
    const char* constant = getConstant(module, arg);
    printf("%s %i (%s)\n", opName, arg, constant);
}

void printModule(Module* module) {
    printf("constants: %i\n", module->constantsLength);
    for(int i = 0; i < module->constantsLength; i++) {
        printf("\t%s\n", module->constants[i]);
    }

    printf("bytecode:\n");
    int i = 0;
    while(i < module->bytecodeLength) {
        printf("\t %i: \t", i);
        unsigned char opcode = module->bytecode[i++];
        if(opcode == OP_PUSH_INT) {
            printf("PUSH_INT %i\n", readInt(module, &i));
        } else if(opcode == OP_PUSH_SYMBOL) {
            printIndexArgOp("PUSH_SYMBOL", module, &i);
        } else if(opcode == OP_PUSH_NONE) {
            printf("PUSH_NONE\n");
        } else if(opcode == OP_CALL) {
            printf("CALL\n");
        } else if(opcode == OP_RETURN) {
            printf("RETURN\n");
        } else if(opcode == OP_CREATE_FUNC) {
            printf("CREATE_FUNC %i\n", readInt(module, &i));
        } else if(opcode == OP_LOAD) {
            printIndexArgOp("LOAD", module, &i);
        } else if(opcode == OP_STORE) {
            printIndexArgOp("STORE", module, &i);
        } else if(opcode == OP_COND_JUMP_TRUE) {
            printf("OP_COND_JUMP_TRUE %i\n", readInt(module, &i));
        } else if(opcode == OP_COND_JUMP_FALSE) {
            printf("OP_COND_JUMP_FALSE %i\n", readInt(module, &i));
        } else if(opcode == OP_ABS_JUMP) {
            printf("OP_ABS_JUMP %i\n", readInt(module, &i));
        } else if(opcode == OP_DEF_FUNC) {
            int argNum = module->bytecode[i++];
            printf("DEF_FUNC %i: ", argNum);
            for(int k = 0; k < argNum; k++) {
                int arg = readInt(module, &i);
                const char* str = getConstant(module, arg);
                printf("%i (%s), ", arg, str);
            }
            printf("\n");
        } else {
            printf("!CORRUPT_BYTECODE!\n");
        }
    }
}

int modulesEqual(Module* a, Module* b) {
    if(a->constantsLength != b->constantsLength) {
        return 0;
    }

    for(int i = 0; i < a->constantsLength; i++) {
        if(strcmp(a->constants[i], b->constants[i]) != 0) {
            return 0;
        }
    }

    if(a->bytecodeLength != b->bytecodeLength) {
        return 0;
    }

    for(int i = 0; i < a->bytecodeLength; i++) {
        if(a->bytecode[i] != b->bytecode[i]) {
            return 0;
        }
    }
    return 1;
}

const char* codegenTestSimple() {
    Token* ast = (Token*) parseModule("def main x do <- 1 + 2; end");
    Module* parsed = compileModule(ast);
    destroyToken(ast);

    ModuleBuilder* builder = createModuleBuilder();

    //global object
    int mainEntry = createLabel(builder);
    emitCreateFunc(builder, mainEntry);
    emitStore(builder, "main");
    emitPushNone(builder);
    emitReturn(builder);

    //main function code
    emitLabel(builder, mainEntry);
    const char* args[1] = {
            "x"
    };
    emitDefFunc(builder, 1, args);
    emitPushInt(builder, 1);
    emitPushSymbol(builder, "+");
    emitCall(builder);
    emitPushInt(builder, 2);
    emitCall(builder);
    emitReturn(builder);
    emitPushNone(builder);
    emitReturn(builder);

    Module* expected = builderToModule(builder);
    destroyModuleBuilder(builder);

    assert(modulesEqual(parsed, expected), "modules not equal");

    destroyModule(parsed);
    destroyModule(expected);

    return NULL;
}

const char* codegenTestJumps() {
    BlockToken* ast = parseModule("def factorial n do if n == 0 then <- 0; else <- n * factorial (n - 1); end end");
    assert(validateModule(ast), "invalid ast");
    Token* transformed = (Token*) transformModule(ast);
    destroyToken((Token*) ast);
    Module* compiled = compileModule(transformed);
    destroyToken((Token*) transformed);

    ModuleBuilder* builder = createModuleBuilder();

    int factorialEntry = createLabel(builder);
    emitCreateFunc(builder, factorialEntry);
    emitStore(builder, "factorial");
    emitPushNone(builder);
    emitReturn(builder);

    emitLabel(builder, factorialEntry);
    const char* args[1] = {
            "n"
    };
    emitDefFunc(builder, 1, args);
    emitLoad(builder, "n");
    emitPushSymbol(builder, "==");
    emitCall(builder);
    emitPushInt(builder, 0);
    emitCall(builder);
    int elseLabel = createLabel(builder);
    emitCondJump(builder, elseLabel, 0);

    emitPushInt(builder, 0);
    emitReturn(builder);
    int endLabel = createLabel(builder);
    emitAbsJump(builder, endLabel);

    emitLabel(builder, elseLabel);
    emitLoad(builder, "n");
    emitPushSymbol(builder, "*");
    emitCall(builder);
    emitLoad(builder, "factorial");
    emitLoad(builder, "n");
    emitPushSymbol(builder, "-");
    emitCall(builder);
    emitPushInt(builder, 1);
    emitCall(builder);
    emitCall(builder);
    emitCall(builder);
    emitReturn(builder);

    emitLabel(builder, endLabel);
    emitPushNone(builder);
    emitReturn(builder);

    Module* expected = builderToModule(builder);
    destroyModuleBuilder(builder);

    assert(modulesEqual(compiled, expected), "modules not equal");

    destroyModule(compiled);
    destroyModule(expected);

    return NULL;
}
