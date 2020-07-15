#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "main/bytecode.h"
#include "main/codegen.h"

//when the module is being compiled, the constant table and the bytecode are
//split into 'segments'. Each segment contains SEGMENT_SIZE elements.
const uint32_t SEGMENT_SIZE = 1024;

ModuleBuilder* createModuleBuilder() {
    ModuleBuilder* builder = (ModuleBuilder*) malloc(sizeof(ModuleBuilder));
    builder->constantsLength = 0;
    builder->constants = NULL;
    builder->bytecodeLength = 0;
    builder->bytecode = NULL;
    builder->srcLocLength = 0;
    builder->srcLoc = NULL;
    builder->nextLabel = 0;
    builder->labelRefs = createMap();
    builder->labelDefs = createMap();
    return builder;
}

void destroyIntList(void* list) {
    destroyList((List*) list, free);
}

void destroyModuleBuilder(ModuleBuilder* builder) {
    builder->constantsLength = 0;
    destroyList(builder->constants, free);
    builder->constants = NULL;

    builder->bytecodeLength = 0;
    destroyList(builder->bytecode, free);
    builder->bytecode = NULL;

    builder->srcLocLength = 0;
    destroyList(builder->srcLoc, free);
    builder->srcLoc = NULL;

    destroyMap(builder->labelRefs, free, destroyIntList);
    destroyMap(builder->labelDefs, free, free);

    free(builder);
}

void emitByte(ModuleBuilder* builder, uint8_t byte) {
    uint32_t segmentOffset = builder->bytecodeLength % SEGMENT_SIZE;
    //if the current bytecode segment has no space left, create a new one
    if(segmentOffset == 0) {
        builder->bytecode = consList(malloc(SEGMENT_SIZE), builder->bytecode);
    }
    unsigned char* currentSegment = (uint8_t*) builder->bytecode->head;
    currentSegment[segmentOffset] = byte;
    builder->bytecodeLength++;
}

void emitInt(ModuleBuilder* builder, int32_t num) {
    emitByte(builder, (num & 0xFF000000) >> 24);
    emitByte(builder, (num & 0x00FF0000) >> 16);
    emitByte(builder, (num & 0x0000FF00) >> 8);
    emitByte(builder, num & 0x000000FF);
}

void emitUInt(ModuleBuilder* builder, uint32_t num) {
    emitByte(builder, (num & 0xFF000000) >> 24);
    emitByte(builder, (num & 0x00FF0000) >> 16);
    emitByte(builder, (num & 0x0000FF00) >> 8);
    emitByte(builder, num & 0x000000FF);
}

//TODO don't assume floats use IEEE-754
void emitFloat(ModuleBuilder* builder, float num) {
    //bitwise cast float to uint32_t
    uint32_t casted = *((uint32_t*) &num);
    emitUInt(builder, casted);
}

/**
 * Returns the index of the given string in the constant table. If the string
 * is not in the constant table, it is added to it. Note that the function does
 * not hold a reference to the constant; it is copied as necessary to the
 * constant table.
 */
uint32_t internConstant(ModuleBuilder* builder, const char* constant) {
    //find the constant and return its index
    uint32_t i = 0;
    for(List* segment = builder->constants; segment != NULL; segment = segment->tail) {
        for(uint32_t k = 0; k < SEGMENT_SIZE; k++) {
            if(i >= builder->constantsLength) {
                break;
            }
            char** segmentData = (char**) segment->head;
            if(strcmp(segmentData[i], constant) == 0) {
                return i;
            }
            i++;
        }
    }

    uint32_t segmentOffset = builder->constantsLength % SEGMENT_SIZE;
    //create a new segment if necessary
    if(segmentOffset == 0) {
        char** newSegment = (char**) malloc(SEGMENT_SIZE * sizeof(char*));
        builder->constants = consList(newSegment, builder->constants);
    }

    //create a copy of the constant and store it if it was not already present
    char** currentSegment = (char**) builder->constants->head;
    currentSegment[segmentOffset] = newStr(constant);
    return builder->constantsLength++;
}

void emitLabelRef(ModuleBuilder* builder, uint32_t label) {
    //record where the label is referenced
    List* value = consList(
            boxUint32(builder->bytecodeLength),
            (List*) getMapUint32(builder->labelRefs, label));
    putMapUint32(builder->labelRefs, label, value);
    //emit a dummy address. This will be patched later, even if the label was
    //already emitted.
    emitUInt(builder, 0);
}

uint32_t createLabel(ModuleBuilder* builder) {
    return builder->nextLabel++;
}

void emitLabel(ModuleBuilder* builder, uint32_t label) {
    putMapUint32(builder->labelDefs, label, boxUint32(builder->bytecodeLength));
}

void emitPushInt(ModuleBuilder* builder, int32_t num) {
    emitByte(builder, OP_PUSH_INT);
    emitInt(builder, num);
}

void emitPushFloat(ModuleBuilder* builder, float num) {
    emitByte(builder, OP_PUSH_FLOAT);
    emitFloat(builder, num);
}

void emitPushBuiltin(ModuleBuilder* builder, const char* name) {
    emitByte(builder, OP_PUSH_BUILTIN);
    emitUInt(builder, internConstant(builder, name));
}

void emitPushLiteral(ModuleBuilder* builder, const char* literal) {
    emitByte(builder, OP_PUSH_LITERAL);
    emitUInt(builder, internConstant(builder, literal));
}

void emitPushNone(ModuleBuilder* builder) {
    emitByte(builder, OP_PUSH_NONE);
}

void emitCall(ModuleBuilder* builder, uint32_t arity) {
    emitByte(builder, OP_CALL);
    emitUInt(builder, arity);
}

void emitReturn(ModuleBuilder* builder) {
    emitByte(builder, OP_RETURN);
}

void emitCreateFunc(ModuleBuilder* builder, uint32_t location) {
    emitByte(builder, OP_CREATE_FUNC);
    emitLabelRef(builder, location);
}

void emitLoad(ModuleBuilder* builder, const char* name) {
    emitByte(builder, OP_LOAD);
    emitUInt(builder, internConstant(builder, name));
}

void emitStore(ModuleBuilder* builder, const char* name) {
    emitByte(builder, OP_STORE);
    emitUInt(builder, internConstant(builder, name));
}

void emitCondJump(ModuleBuilder* builder, uint32_t label, uint8_t when) {
    if(when) {
        emitByte(builder, OP_COND_JUMP_TRUE);
    } else {
        emitByte(builder, OP_COND_JUMP_FALSE);
    }
    emitLabelRef(builder, label);
}

void emitAbsJump(ModuleBuilder* builder, uint32_t label) {
    emitByte(builder, OP_ABS_JUMP);
    emitLabelRef(builder, label);
}

void emitDup(ModuleBuilder* builder) {
    emitByte(builder, OP_DUP);
}

void emitRot3(ModuleBuilder* builder) {
    emitByte(builder, OP_ROT3);
}

void emitSwap(ModuleBuilder* builder) {
    emitByte(builder, OP_SWAP);
}

void emitPop(ModuleBuilder* builder) {
    emitByte(builder, OP_POP);
}

void emitCheckNone(ModuleBuilder* builder) {
    emitByte(builder, OP_CHECK_NONE);
}

void emitDefFunc(ModuleBuilder* builder, uint8_t argNum, const char** args,
        uint8_t isInit) {
    if(!isInit) {
        emitByte(builder, OP_DEF_FUNC);
        emitByte(builder, argNum);
        for(uint8_t i = 0; i < argNum; i++) {
            emitUInt(builder, internConstant(builder, args[i]));
        }
    }
}

void emitSrcLoc(ModuleBuilder* builder, SrcLoc location) {
    uint32_t segmentOffset = builder->srcLocLength % SEGMENT_SIZE;
    //if the current bytecode segment has no space left, create a new one
    if(segmentOffset == 0) {
        size_t size = SEGMENT_SIZE * sizeof(BytecodeSrcLoc);
        builder->srcLoc = consList(malloc(size), builder->srcLoc);
    }
    BytecodeSrcLoc* currentSegment = (BytecodeSrcLoc*) builder->srcLoc->head;
    uint32_t index = builder->bytecodeLength;
    currentSegment[segmentOffset] = createBytecodeSrcLoc(index, location);
    builder->srcLocLength++;
}

/**
 * Takes a list of segments and copies them to a single array. Each segment is
 * assumed to be filled to the capacity of SEGMENT_SIZE, except the first one
 * (which occurs at the end of the bytecode).
 *
 * @param numElems the number of elements in all the segments.
 * @param segment list of segments
 * @param elemSize the size of each element
 * @return an array of compacted elements
 */
char* compactSegments(uint32_t numElems, List* segment, uint8_t elemSize) {
    //reverse the segments to the order that they actually occur in
    List* revSegment = reverseList(segment);
    segment = revSegment;

    //size of the compacted array in bytes
    uint32_t length = numElems * elemSize;

    //size of each segment in bytes
    uint32_t segmentSize = SEGMENT_SIZE * elemSize;
    char* compacted = (char*) malloc(length);
    uint32_t i;
    for(i = 0; i + segmentSize < length; i += segmentSize) {
        memcpy(&compacted[i], segment->head, segmentSize);
        segment = segment->tail;
    }
    if(segment != NULL) {
        memcpy(&compacted[i], segment->head, length % segmentSize);
    }

    destroyShallowList(revSegment);
    return compacted;
}

Module* builderToModule(ModuleBuilder* builder, uint32_t entryLabel) {
    char** constants = (char**) compactSegments(builder->constantsLength,
            builder->constants, sizeof(char*));
    uint8_t* bytecode = (unsigned char*) compactSegments(
            builder->bytecodeLength, builder->bytecode, sizeof(unsigned char));
    BytecodeSrcLoc* srcLoc = (BytecodeSrcLoc*) compactSegments(builder->srcLocLength,
            builder->srcLoc, sizeof(BytecodeSrcLoc));

    //patch the labels
    for(Entry* i = builder->labelRefs->entry; i != NULL; i = i->tail) {
        //TODO change ot getMapUint32_t
        uint32_t* definition = (uint32_t*) getMapStr(builder->labelDefs, (char*) i->key);
        for(List* k = (List*) i->value; k != NULL; k = k->tail) {
            uint32_t reference = *((uint32_t*) k->head);
            bytecode[reference] = (*definition & 0xFF000000) >> 24;
            bytecode[reference + 1] = (*definition & 0x00FF0000) >> 16;
            bytecode[reference + 2] = (*definition & 0x0000FF00) >> 8;
            bytecode[reference + 3] = *definition & 0x000000FF;
        }
    }

    Module* module = (Module*) malloc(sizeof(Module));
    module->constantsLength = builder->constantsLength;
    module->constants = (const char**) constants;
    module->bytecodeLength = builder->bytecodeLength;
    module->bytecode = bytecode;
    module->srcLocLength = builder->srcLocLength;
    module->srcLoc = srcLoc;
    uint32_t* entryIndex = (uint32_t*) getMapUint32(builder->labelDefs, entryLabel);
    module->entryIndex = *entryIndex;
    module->name = NULL;
    return module;
}

/**
 * Converts statement, expression and jump tokens into bytecode.
 * Recursively compiles tokens.
 *
 * @param builder the ModuleBuilder instance
 * @param labels maps local label names to label integer values generated by
 *          createLabel
 * @param token the token to compile
 */
void compileToken(ModuleBuilder* builder, Map* globalFuncs, Map* labels,
        Token* token) {
    if(getTokenType(token) == TOKEN_INT) {
        emitSrcLoc(builder, tokenLocation(token));
        emitPushInt(builder, getIntTokenValue((IntToken*) token));
    } else if(getTokenType(token) == TOKEN_FLOAT) {
        emitSrcLoc(builder, tokenLocation(token));
        emitPushFloat(builder, (getFloatTokenValue((FloatToken*) token)));
    } else if(getTokenType(token) == TOKEN_LITERAL) {
        emitSrcLoc(builder, tokenLocation(token));
        emitPushLiteral(builder, getLiteralTokenValue((LiteralToken*) token));
    } else if(getTokenType(token) == TOKEN_IDENTIFIER) {
        emitSrcLoc(builder, tokenLocation(token));
        emitLoad(builder, getIdentifierTokenValue((IdentifierToken*) token));
    } else if(getTokenType(token) == TOKEN_LABEL) {
        uint32_t* label = (uint32_t*) getMapStr(labels, ((LabelToken*) token)->name);
        emitLabel(builder, *label);
    } else if(getTokenType(token) == TOKEN_ABS_JUMP) {
        uint32_t* label = (uint32_t*) getMapStr(labels, ((AbsJumpToken*) token)->label);
        emitSrcLoc(builder, tokenLocation(token));
        emitAbsJump(builder, *label);
    } else if(getTokenType(token) == TOKEN_COND_JUMP) {
        CondJumpToken* condJump = (CondJumpToken*) token;
        compileToken(builder, globalFuncs, labels, condJump->condition);

        uint32_t* label = (uint32_t*) getMapStr(labels, condJump->label);
        emitSrcLoc(builder, tokenLocation(token));
        emitCondJump(builder, *label, condJump->when);
    } else if(getTokenType(token) == TOKEN_TUPLE) {
        emitSrcLoc(builder, tokenLocation(token));
        emitPushBuiltin(builder, "tuple");

        TupleToken* tuple = (TupleToken*) token;
        List* elements = getTupleTokenElements(tuple);
        uint8_t count = 0;

        while(elements != NULL) {
            compileToken(builder, globalFuncs, labels, (Token*) elements->head);
            elements = elements->tail;
            count++;
        }

        emitSrcLoc(builder, tokenLocation(token));
        emitCall(builder, count);
    } else if(getTokenType(token) == TOKEN_CALL) {
        CallToken* call = (CallToken*) token;
        List* children = getCallTokenChildren(call);
        uint32_t count = 0;
        while(children != NULL) {
            compileToken(builder, globalFuncs, labels, (Token*) children->head);
            children = children->tail;
            count++;
        }

        emitSrcLoc(builder, tokenLocation(token));
        emitCall(builder, count - 1);
    } else if(getTokenType(token) == TOKEN_UNARY_OP) {
        emitSrcLoc(builder, tokenLocation(token));
        UnaryOpToken* unaryOp = (UnaryOpToken*) token;
        emitPushBuiltin(builder, getUnaryOpTokenOp(unaryOp));

        compileToken(builder, globalFuncs, labels, getUnaryOpTokenChild(unaryOp));

        emitSrcLoc(builder, tokenLocation(token));
        emitCall(builder, 1);
    } else if(getTokenType(token) == TOKEN_BINARY_OP) {
        emitSrcLoc(builder, tokenLocation(token));
        BinaryOpToken* binaryOp = (BinaryOpToken*) token;
        emitPushBuiltin(builder, getBinaryOpTokenOp(binaryOp));
        compileToken(builder, globalFuncs, labels, getBinaryOpTokenLeft(binaryOp));
        compileToken(builder, globalFuncs, labels, getBinaryOpTokenRight(binaryOp));

        emitSrcLoc(builder, tokenLocation(token));
        emitCall(builder, 2);
    } else if(getTokenType(token) == TOKEN_RETURN) {
        ReturnToken* ret = (ReturnToken*) token;
        compileToken(builder, globalFuncs, labels, ret->body);

        emitSrcLoc(builder, tokenLocation(token));
        emitReturn(builder);
    } else if(getTokenType(token) == TOKEN_PUSH_BUILTIN) {
        emitSrcLoc(builder, tokenLocation(token));
        emitPushBuiltin(builder, ((PushBuiltinToken*) token)->name);
    } else if(getTokenType(token) == TOKEN_PUSH_INT) {
        emitSrcLoc(builder, tokenLocation(token));
        emitPushInt(builder, ((PushIntToken*) token)->value);
    } else if(getTokenType(token) == TOKEN_OP_CALL) {
        emitSrcLoc(builder, tokenLocation(token));
        emitCall(builder, ((CallOpToken*) token)->arity);
    } else if(getTokenType(token) == TOKEN_STORE) {
        emitSrcLoc(builder, tokenLocation(token));
        emitStore(builder, ((StoreToken*) token)->name);
    } else if(getTokenType(token) == TOKEN_DUP) {
        emitSrcLoc(builder, tokenLocation(token));
        emitDup(builder);
    } else if(getTokenType(token) == TOKEN_PUSH) {
        emitSrcLoc(builder, tokenLocation(token));
        compileToken(builder, globalFuncs, labels, ((PushToken*) token)->value);
    } else if(getTokenType(token) == TOKEN_ROT3) {
        emitSrcLoc(builder, tokenLocation(token));
        emitRot3(builder);
    } else if(getTokenType(token) == TOKEN_SWAP) {
        emitSrcLoc(builder, tokenLocation(token));
        emitSwap(builder);
    } else if(getTokenType(token) == TOKEN_POP) {
        emitSrcLoc(builder, tokenLocation(token));
        emitPop(builder);
    } else if(getTokenType(token) == TOKEN_BUILTIN) {
        emitSrcLoc(builder, tokenLocation(token));
        emitPushBuiltin(builder, ((BuiltinToken*) token)->name);
    } else if(getTokenType(token) == TOKEN_CHECK_NONE) {
        emitSrcLoc(builder, tokenLocation(token));
        emitCheckNone(builder);
    } else if(getTokenType(token) == TOKEN_NEW_FUNC) {
        emitSrcLoc(builder, tokenLocation(token));
        uint32_t* label = (uint32_t*) getMapStr(globalFuncs, ((NewFuncToken*) token)->name);
        emitCreateFunc(builder, *label);
    } else {
        printf("warning: unknown token type\n");
    }
}

/**
 * Compiles a function token.
 *
 * @param builder the ModuleBuilder instance
 * @param globalFuncs maps function names to function entry labels
 * @param func the function to compile
 */
void compileFunc(ModuleBuilder* builder, Map* globalFuncs, FuncToken* func) {
    //record the start of the function
    emitLabel(builder, *((uint32_t*) getMapStr(globalFuncs, getIdentifierTokenValue(func->name))));

    //convert the argument list to an array be passed to emitDefFunc
    uint8_t argNum = lengthList(func->args);
    char** args = (char**) malloc(sizeof(char*) * lengthList(func->args));
    List* arg = func->args;
    for(uint8_t i = 0; i < argNum; i++) {
        args[i] = (char*) getIdentifierTokenValue((IdentifierToken*) arg->head);
        arg = arg->tail;
    }
    //indicate the beginning of the function
    emitSrcLoc(builder, tokenLocation((Token*) func));
    uint8_t isInit = strcmp(getIdentifierTokenValue(func->name), "$init") == 0;
    emitDefFunc(builder, argNum, (const char**) args, isInit);
    free(args);

    //create a label for each LabelToken and map its name to the value
    //associated with the ModuleBuilder
    Map* labels = createMap();
    for(List* list = func->body->children; list != NULL; list = list->tail) {
        Token* token = (Token*) list->head;
        if(getTokenType(token) == TOKEN_LABEL) {
            LabelToken* label = (LabelToken*) token;
            putMapStr(labels, label->name, boxUint32(createLabel(builder)));
        }
    }

    //finally, generate each statement / jump
    for(List* list = func->body->children; list != NULL; list = list->tail) {
        compileToken(builder, globalFuncs, labels, (Token*) list->head);
    }

    destroyMap(labels, nothing, free);

    emitPushNone(builder);
    emitReturn(builder);
}

Module* compileModule(Token* ast) {
    ModuleBuilder* builder = createModuleBuilder();

    //globalFuncs maps function names to labels
    Map* globalFuncs = createMap();
    BlockToken* block = (BlockToken*) ast;
    //create the module object / global scope as the local scope
    for(List* list = block->children; list != NULL; list = list->tail) {
        Token* token = (Token*) list->head;
        if(getTokenType(token) == TOKEN_FUNC) {
            FuncToken* func = (FuncToken*) token;
            uint32_t label = createLabel(builder);
            putMapStr(globalFuncs, newStr(getIdentifierTokenValue(func->name)), boxUint32(label));

            //emitSrcLoc(builder, func->token.location);
            //emitCreateFunc(builder, label);

            //emitSrcLoc(builder, func->token.location);
            //emitStore(builder, func->name->value);
        }
    }

    //emitLoad(builder, "$init");
    //emitPushBuiltin(builder, "none");
    //emitCall(builder, 1);

    //emitPushNone(builder);
    //emitReturn(builder);

    //compile each function
    for(List* list = block->children; list != NULL; list = list->tail) {
        Token* token = (Token*) list->head;
        if(getTokenType(token) == TOKEN_FUNC) {
            compileFunc(builder, globalFuncs, (FuncToken*) token);
        }
    }

    uint32_t* labelEntry = (uint32_t*) getMapStr(globalFuncs, "$init");
    Module* module = builderToModule(builder, *labelEntry);
    destroyModuleBuilder(builder);
    destroyMap(globalFuncs, free, free);
    return module;
}

void destroyModule(Module* module) {
    for(uint32_t i = 0; i < module->constantsLength; i++) {
        free((void*) module->constants[i]);
    }
    free((void*) module->constants);
    module->constants = NULL;
    module->constantsLength = 0;

    free((void*) module->bytecode);
    module->bytecode = NULL;
    module->bytecodeLength = 0;

    free((void*) module->srcLoc);
    module->srcLoc = NULL;
    module->srcLocLength = 0;

    free(module);
}
