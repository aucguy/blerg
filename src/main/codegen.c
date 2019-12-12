#include <stdlib.h>
#include <string.h>

#include "main/codegen.h"

//when the module is being compiled, the constant table and the bytecode are
//split into 'segments'. Each segment contains SEGMENT_SIZE elements.
const int SEGMENT_SIZE = 1024;

ModuleBuilder* createModuleBuilder() {
    ModuleBuilder* builder = (ModuleBuilder*) malloc(sizeof(ModuleBuilder));
    builder->constantsLength = 0;
    builder->constants = NULL;
    builder->bytecodeLength = 0;
    builder->bytecode = NULL;
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

    destroyMap(builder->labelRefs, free, destroyIntList);
    destroyMap(builder->labelDefs, free, free);

    free(builder);
}

void emitByte(ModuleBuilder* builder, unsigned char byte) {
    int segmentOffset = builder->bytecodeLength % SEGMENT_SIZE;
    //if the current bytecode segment has no space left, create a new one
    if(segmentOffset == 0) {
        builder->bytecode = consList(malloc(SEGMENT_SIZE), builder->bytecode);
    }
    unsigned char* currentSegment = (unsigned char*) builder->bytecode->head;
    currentSegment[segmentOffset] = byte;
    builder->bytecodeLength++;
}

void emitInt(ModuleBuilder* builder, int num) {
    emitByte(builder, (num & 0xFF000000) >> 24);
    emitByte(builder, (num & 0x00FF0000) >> 16);
    emitByte(builder, (num & 0x0000FF00) >> 8);
    emitByte(builder, num & 0x000000FF);
}

void emitUInt(ModuleBuilder* builder, unsigned int num) {
    emitByte(builder, (num & 0xFF000000) >> 24);
    emitByte(builder, (num & 0x00FF0000) >> 16);
    emitByte(builder, (num & 0x0000FF00) >> 8);
    emitByte(builder, num & 0x000000FF);
}

/**
 * Returns the index of the given string in the constant table. If the string
 * is not in the constant table, it is added to it. Note that the function does
 * not hold a reference to the constant; it is copied as necessary to the
 * constant table.
 */
unsigned int internConstant(ModuleBuilder* builder, const char* constant) {
    //find the constant and return its index
    unsigned int i = 0;
    for(List* segment = builder->constants; segment != NULL; segment = segment->tail) {
        for(int k = 0; k < SEGMENT_SIZE; k++) {
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

    int segmentOffset = builder->constantsLength % SEGMENT_SIZE;
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

void emitLabelRef(ModuleBuilder* builder, int label) {
    //record where the label is referenced
    List* value = consList(
            boxInt(builder->bytecodeLength),
            (List*) getMapInt(builder->labelRefs, label));
    putMapInt(builder->labelRefs, label, value);
    //emit a dummy address. This will be patched later, even if the label was
    //already emitted.
    emitUInt(builder, 0);
}

int createLabel(ModuleBuilder* builder) {
    return builder->nextLabel++;
}

void emitLabel(ModuleBuilder* builder, int label) {
    putMapInt(builder->labelDefs, label, boxInt(builder->bytecodeLength));
}

void emitPushInt(ModuleBuilder* builder, int num) {
    emitByte(builder, OP_PUSH_INT);
    emitInt(builder, num);
}

void emitPushSymbol(ModuleBuilder* builder, const char* symbol) {
    emitByte(builder, OP_PUSH_SYMBOL);
    emitUInt(builder, internConstant(builder, symbol));
}

void emitPushLiteral(ModuleBuilder* builder, const char* literal) {
    emitByte(builder, OP_PUSH_LITERAL);
    emitUInt(builder, internConstant(builder, literal));
}

void emitPushNone(ModuleBuilder* builder) {
    emitByte(builder, OP_PUSH_NONE);
}

void emitCall(ModuleBuilder* builder) {
    emitByte(builder, OP_CALL);
}

void emitReturn(ModuleBuilder* builder) {
    emitByte(builder, OP_RETURN);
}

void emitCreateFunc(ModuleBuilder* builder, int location) {
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

void emitCondJump(ModuleBuilder* builder, int label, int when) {
    if(when) {
        emitByte(builder, OP_COND_JUMP_TRUE);
    } else {
        emitByte(builder, OP_COND_JUMP_FALSE);
    }
    emitLabelRef(builder, label);
}

void emitAbsJump(ModuleBuilder* builder, int label) {
    emitByte(builder, OP_ABS_JUMP);
    emitLabelRef(builder, label);
}

void emitDefFunc(ModuleBuilder* builder, char argNum, const char** args) {
    emitByte(builder, OP_DEF_FUNC);
    emitByte(builder, argNum);
    for(int i = 0; i < argNum; i++) {
        emitUInt(builder, internConstant(builder, args[i]));
    }
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
char* compactSegments(int numElems, List* segment, int elemSize) {
    //reverse the segments to the order that they actually occur in
    List* revSegment = reverseList(segment);
    segment = revSegment;

    //size of the compacted array in bytes
    int length = numElems * elemSize;

    //size of each segment in bytes
    int segmentSize = SEGMENT_SIZE * elemSize;
    char* compacted = (char*) malloc(length);
    int i;
    for(i = 0; i < length - segmentSize; i += segmentSize) {
        memcpy(&compacted[i], segment->head, segmentSize);
        segment = segment->tail;
    }
    if(segment != NULL) {
        memcpy(&compacted[i], segment->head, length % segmentSize);
    }

    destroyShallowList(revSegment);
    return compacted;
}

Module* builderToModule(ModuleBuilder* builder) {
    char** constants = (char**) compactSegments(builder->constantsLength,
            builder->constants, sizeof(char*));
    unsigned char* bytecode = (unsigned char*) compactSegments(
            builder->bytecodeLength, builder->bytecode, sizeof(unsigned char));

    //patch the labels
    for(Entry* i = builder->labelRefs->entry; i != NULL; i = i->tail) {
        unsigned int* definition = getMapStr(builder->labelDefs, (char*) i->key);
        for(List* k = (List*) i->value; k != NULL; k = k->tail) {
            unsigned int reference = *((int*) k->head);
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
void compileToken(ModuleBuilder* builder, Map* labels, Token* token) {
    if(token->type == TOKEN_INT) {
        emitPushInt(builder, ((IntToken*) token)->value);
    } else if(token->type == TOKEN_LITERAL) {
        emitPushLiteral(builder, ((LiteralToken*) token)->value);
    } else if(token->type == TOKEN_IDENTIFIER) {
        emitLoad(builder, ((IdentifierToken*) token)->value);
    } else if(token->type == TOKEN_LABEL) {
        int* label = (int*) getMapStr(labels, ((LabelToken*) token)->name);
        emitLabel(builder, *label);
    } else if(token->type == TOKEN_ABS_JUMP) {
        int* label = (int*) getMapStr(labels, ((AbsJumpToken*) token)->label);
        emitAbsJump(builder, *label);
    } else if(token->type == TOKEN_COND_JUMP) {
        CondJumpToken* condJump = (CondJumpToken*) token;
        compileToken(builder, labels, condJump->condition);
        int* label = (int*) getMapStr(labels, condJump->label);
        emitCondJump(builder, *label, condJump->when);
    } else if(token->type == TOKEN_UNARY_OP) {
        UnaryOpToken* unaryOp = (UnaryOpToken*) token;
        compileToken(builder, labels, unaryOp->child);
        emitPushSymbol(builder, unaryOp->op);
        emitCall(builder);
    } else if(token->type == TOKEN_BINARY_OP) {
        BinaryOpToken* binaryOp = (BinaryOpToken*) token;
        compileToken(builder, labels, binaryOp->left);
        //the " " operator is really just a function call, so it does not
        //need to be passed to the left operand
        if(strcmp(binaryOp->op, " ") != 0) {
            emitPushSymbol(builder, binaryOp->op);
            emitCall(builder);
        }
        compileToken(builder, labels, binaryOp->right);
        emitCall(builder);
    } else if(token->type == TOKEN_RETURN) {
        ReturnToken* ret = (ReturnToken*) token;
        compileToken(builder, labels, ret->body);
        emitReturn(builder);
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
    emitLabel(builder, *((int*) getMapStr(globalFuncs, func->name->value)));

    //convert the argument list to an array be passed to emitDefFunc
    int argNum = lengthList(func->args);
    char** args = (char**) malloc(sizeof(char*) * lengthList(func->args));
    List* arg = func->args;
    for(int i = 0; i < argNum; i++) {
        args[i] = (char*) ((IdentifierToken*) arg->head)->value;
        arg = func->args;
    }
    //indicate the beginning of the function
    emitDefFunc(builder, argNum, (const char**) args);
    free(args);

    //create a label for each LabelToken and map its name to the value
    //associated with the ModuleBuilder
    Map* labels = createMap();
    for(List* list = func->body->children; list != NULL; list = list->tail) {
        Token* token = (Token*) list->head;
        if(token->type == TOKEN_LABEL) {
            LabelToken* label = (LabelToken*) token;
            putMapStr(labels, label->name, boxInt(createLabel(builder)));
        }
    }

    //finally, generate each statement / jump
    for(List* list = func->body->children; list != NULL; list = list->tail) {
        compileToken(builder, labels, (Token*) list->head);
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
        if(token->type == TOKEN_FUNC) {
            FuncToken* func = (FuncToken*) token;
            int label = createLabel(builder);
            putMapStr(globalFuncs, newStr(func->name->value), boxInt(label));
            emitCreateFunc(builder, label);
            emitStore(builder, func->name->value);
        }
    }

    emitPushNone(builder);
    emitReturn(builder);

    //compile each function
    for(List* list = block->children; list != NULL; list = list->tail) {
        Token* token = (Token*) list->head;
        if(token->type == TOKEN_FUNC) {
            compileFunc(builder, globalFuncs, (FuncToken*) token);
        }
    }

    Module* module = builderToModule(builder);
    destroyModuleBuilder(builder);
    destroyMap(globalFuncs, free, free);
    return module;
}

void destroyModule(Module* module) {
    for(unsigned int i = 0; i < module->constantsLength; i++) {
        free((void*) module->constants[i]);
    }
    free((void*) module->constants);
    module->constantsLength = 0;

    free((void*) module->bytecode);
    module->bytecodeLength = 0;

    free(module);
}
