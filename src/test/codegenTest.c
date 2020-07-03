#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "main/parse.h"
#include "main/validate.h"
#include "main/transform.h"
#include "main/bytecode.h"
#include "main/codegen.h"

#include "test/tests.h"

int32_t readInt(Module* module, uint32_t* index) {
    int32_t arg = module->bytecode[*index] << 24;
    arg |= module->bytecode[*index + 1] << 16;
    arg |= module->bytecode[*index + 2] << 8;
    arg |= module->bytecode[*index + 3];
    *index += 4;
    return arg;
}

uint32_t readUInt(Module* module, uint32_t* index) {
    uint32_t arg = module->bytecode[*index] << 24;
    arg |= module->bytecode[*index + 1] << 16;
    arg |= module->bytecode[*index + 2] << 8;
    arg |= module->bytecode[*index + 3];
    *index += 4;
    return arg;
}

float readFloat(Module* module, uint32_t* index) {
    //TODO don't assume float is not in the IEEE-754 format?
    uint32_t arg = readUInt(module, index);
    return *((float*) &arg);
}

const char* getConstant(Module* module, uint32_t arg) {
    if(arg < module->constantsLength) {
        return module->constants[arg];
    } else {
        return "!corrupt bytecode!";
    }
}

void printIndexArgOp(const char* opName, Module* module, uint32_t* index) {
    uint32_t arg = readUInt(module, index);
    const char* constant = getConstant(module, arg);
    printf("%s %i (%s)", opName, arg, constant);
}

void printModule(Module* module) {
    printf("constants: %i\n", module->constantsLength);
    for(uint32_t i = 0; i < module->constantsLength; i++) {
        printf("\t%s\n", module->constants[i]);
    }

    printf("bytecode:\n");
    printf("\ti\tl\tc\n");
    uint32_t i = 0;
    while(i < module->bytecodeLength) {
        uint32_t srcIndex = 0;
        while(srcIndex < module->srcLocLength && module->srcLoc[srcIndex].index != i) {
            srcIndex++;
        }
        SrcLoc location;
        if(srcIndex == module->srcLocLength) {
            location.line = 0;
            location.column = 0;
        } else {
            location = module->srcLoc[srcIndex].location;
        }
        printf("\t%i\t%i\t%i:\t", i, location.line, location.column);
        unsigned char opcode = module->bytecode[i++];
        if(opcode == OP_PUSH_INT) {
            printf("PUSH_INT %i", readInt(module, &i));
        } else if(opcode == OP_PUSH_FLOAT) {
            printf("PUSH_FLOAT %f", readFloat(module, &i));
        } else if(opcode == OP_PUSH_BUILTIN) {
            printIndexArgOp("PUSH_BUILTIN", module, &i);
        } else if(opcode == OP_PUSH_LITERAL) {
            printIndexArgOp("PUSH_LITERAL", module, &i);
        } else if(opcode == OP_PUSH_NONE) {
            printf("PUSH_NONE");
        } else if(opcode == OP_CALL) {
            printf("CALL %i", readUInt(module, &i));
        } else if(opcode == OP_RETURN) {
            printf("RETURN");
        } else if(opcode == OP_CREATE_FUNC) {
            printf("CREATE_FUNC %i", readInt(module, &i));
        } else if(opcode == OP_LOAD) {
            printIndexArgOp("LOAD", module, &i);
        } else if(opcode == OP_STORE) {
            printIndexArgOp("STORE", module, &i);
        } else if(opcode == OP_COND_JUMP_TRUE) {
            printf("OP_COND_JUMP_TRUE %i", readInt(module, &i));
        } else if(opcode == OP_COND_JUMP_FALSE) {
            printf("OP_COND_JUMP_FALSE %i", readInt(module, &i));
        } else if(opcode == OP_ABS_JUMP) {
            printf("OP_ABS_JUMP %i", readInt(module, &i));
        } else if(opcode == OP_DEF_FUNC) {
            uint8_t argNum = module->bytecode[i++];
            if(opcode == OP_DEF_FUNC) {
                printf("DEF_FUNC %i: ", argNum);
            } else {
                printf("DEF_FUNC_INIT %i: ", argNum);
            }
            for(uint8_t k = 0; k < argNum; k++) {
                uint32_t arg = readUInt(module, &i);
                const char* str = getConstant(module, arg);
                printf("%i (%s), ", arg, str);
            }
        } else if(opcode == OP_DUP) {
            printf("DUP");
        } else if(opcode == OP_ROT3) {
            printf("ROT3");
        } else if(opcode == OP_SWAP) {
            printf("SWAP");
        } else if(opcode == OP_POP) {
            printf("POP");
        } else if(opcode == OP_CHECK_NONE) {
            printf("CHECK_NONE");
        } else {
            printf("!CORRUPT_BYTECODE!\n");
        }
        printf("\n");
    }
}

uint8_t modulesEqual(Module* a, Module* b) {
    if(a->constantsLength != b->constantsLength) {
        return 0;
    }

    for(uint32_t i = 0; i < a->constantsLength; i++) {
        if(strcmp(a->constants[i], b->constants[i]) != 0) {
            return 0;
        }
    }

    if(a->bytecodeLength != b->bytecodeLength) {
        return 0;
    }

    for(uint32_t i = 0; i < a->bytecodeLength; i++) {
        if(a->bytecode[i] != b->bytecode[i]) {
            return 0;
        }
    }
    return 1;
}

const char* codegenTestSimple() {
    char* error;
    Token* ast = (Token*) parseModule("main = def x do return 1 + 2; end;", &error);
    assert(ast != NULL, "incorrect parse");
    Token* transformed = (Token*) transformModule((BlockToken*) ast);
    Module* parsed = compileModule(transformed);
    destroyToken(ast);
    destroyToken(transformed);

    ModuleBuilder* builder = createModuleBuilder();

    uint32_t initLabel = createLabel(builder);
    emitLabel(builder, initLabel);

    //global object
    uint32_t mainEntry = createLabel(builder);
    emitCreateFunc(builder, mainEntry);
    emitStore(builder, "main");
    emitPushNone(builder);
    emitReturn(builder);

    //main function code
    emitLabel(builder, mainEntry);
    const char* args[1] = {
            "x"
    };
    emitDefFunc(builder, 1, args, 0);
    emitPushBuiltin(builder, "+");
    emitPushInt(builder, 1);
    emitPushInt(builder, 2);
    emitCall(builder, 2);
    emitReturn(builder);
    emitPushNone(builder);
    emitReturn(builder);

    Module* expected = builderToModule(builder, initLabel);
    destroyModuleBuilder(builder);

    assert(modulesEqual(parsed, expected), "modules not equal");

    destroyModule(parsed);
    destroyModule(expected);

    return NULL;
}

const char* codegenTestJumps() {
    char* error;
    BlockToken* ast = parseModule("factorial = def n do if n == 0 then return 0; else return n * factorial (n - 1); end end;", &error);
    assert(ast != NULL, "incorrect parse");
    assert(validateModule(ast), "invalid ast");
    Token* transformed = (Token*) transformModule(ast);
    destroyToken((Token*) ast);
    Module* compiled = compileModule(transformed);
    destroyToken((Token*) transformed);

    ModuleBuilder* builder = createModuleBuilder();

    uint32_t factorialEntry = createLabel(builder);
    emitCreateFunc(builder, factorialEntry);
    emitStore(builder, "factorial");
    emitPushNone(builder);
    emitReturn(builder);

    emitLabel(builder, factorialEntry);
    const char* args[1] = {
            "n"
    };
    emitDefFunc(builder, 1, args, 0);
    emitPushBuiltin(builder, "==");
    emitLoad(builder, "n");
    emitPushInt(builder, 0);
    emitCall(builder, 2);
    uint32_t elseLabel = createLabel(builder);
    emitCondJump(builder, elseLabel, 0);

    emitPushInt(builder, 0);
    emitReturn(builder);
    uint32_t endLabel = createLabel(builder);
    emitAbsJump(builder, endLabel);

    emitLabel(builder, elseLabel);
    emitPushBuiltin(builder, "*");
    emitLoad(builder, "n");
    emitLoad(builder, "factorial");
    emitPushBuiltin(builder, "-");
    emitLoad(builder, "n");
    emitPushInt(builder, 1);
    emitCall(builder, 2);
    emitCall(builder, 1);
    emitCall(builder, 2);
    emitReturn(builder);

    emitLabel(builder, endLabel);
    emitPushNone(builder);
    emitReturn(builder);

    Module* expected = builderToModule(builder, 0);
    destroyModuleBuilder(builder);

    assert(modulesEqual(compiled, expected), "modules not equal");

    destroyModule(compiled);
    destroyModule(expected);

    return NULL;
}

const char* codegenTestLiteralUnaryOp() {
    char* error = NULL;
    BlockToken* ast = parseModule("main = def x do return not 'hello'; end;", &error);
    assert(ast != NULL, "incorrect parse");
    assert(validateModule(ast), "invalid ast");
    Token* transformed = (Token*) transformModule(ast);
    destroyToken((Token*) ast);
    Module* compiled = compileModule(transformed);
    destroyToken(transformed);

    ModuleBuilder* builder = createModuleBuilder();

    uint32_t initLabel = createLabel(builder);
    emitLabel(builder, initLabel);

    uint32_t mainEntry = createLabel(builder);
    emitCreateFunc(builder, mainEntry);
    emitStore(builder, "main");
    emitPushNone(builder);
    emitReturn(builder);

    emitLabel(builder, mainEntry);
    const char* args[1] = {
            "x"
    };
    emitDefFunc(builder, 1, args, 0);
    emitPushBuiltin(builder, "not");
    emitPushLiteral(builder, "hello");
    emitCall(builder, 1);
    emitReturn(builder);

    emitPushNone(builder);
    emitReturn(builder);

    Module* expected = builderToModule(builder, initLabel);
    destroyModuleBuilder(builder);

    assert(modulesEqual(compiled, expected), "modules not equal");

    destroyModule(compiled);
    destroyModule(expected);

    return NULL;
}
