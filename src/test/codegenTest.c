#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main/parse.h"
#include "main/codegen.h"

#include "test/tests.h"

int readInt(Module* module, int* index) {
    int arg = module->bytecode[*index];
    arg |= module->bytecode[*index + 1] << 8;
    arg |= module->bytecode[*index + 2] << 16;
    arg |= module->bytecode[*index + 3] << 24;
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

void printModule(Module* module) {
    printf("constants: %i\n", module->constantsLength);
    for(int i = 0; i < module->constantsLength; i++) {
        printf("\t%s\n", module->constants[i]);
    }

    printf("bytecode:\n");
    int i = 0;
    while(i < module->bytecodeLength) {
        printf("\t %i: \t", i);
        char opcode = module->bytecode[i++];
        if(opcode == OP_PUSH_INT) {
            printf("PUSH_INT %i\n", readInt(module, &i));
        } else if(opcode == OP_PUSH_SYMBOL) {
            int arg = readInt(module, &i);
            const char* str = getConstant(module, arg);
            printf("PUSH_SYMBOL %i (%s)\n", arg, str);
        } else if(opcode == OP_PUSH_NONE) {
            printf("PUSH_NONE\n");
        } else if(opcode == OP_CALL) {
            printf("CALL\n");
        } else if(opcode == OP_RETURN) {
            printf("RETURN\n");
        } else if(opcode == OP_CREATE_FUNC) {
            printf("CREATE_FUNC %i\n", readInt(module, &i));
        } else if(opcode == OP_STORE) {
            int arg = readInt(module, &i);
            const char* str = getConstant(module, arg);
            printf("STORE %i (%s)\n", arg, str);
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

const char* codegenTest() {
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

    assert(parsed->constantsLength == expected->constantsLength,
            "constant arrays not equal");

    for(int i = 0; i < parsed->constantsLength; i++) {
        const char* a = parsed->constants[i];
        const char* b = parsed->constants[i];
        assert(strcmp(a, b) == 0, "constant arrays not equal");
    }

    assert(parsed->bytecodeLength == parsed->bytecodeLength,
            "bytecode not equal");

    for(int i = 0; i < parsed->bytecodeLength; i++) {
        assert(parsed->bytecode[i] == expected->bytecode[i],
                "bytecode not equal");
    }

    destroyModule(parsed);
    destroyModule(expected);

    return NULL;
}
