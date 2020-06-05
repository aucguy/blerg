#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "main/util.h"
#include "main/transform.h"

#define UNUSED(x) (void)(x)

//TODO flatten PushTokens

/**
 * Used to create a unique name. This function takes an integer, concatenates
 * it with '$' and increments uniqueId.
 *
 * TODO use uint32_t instead
 */
const char* uniqueName(uint8_t* uniqueId) {
    int length = snprintf(NULL, 0, "$%d", *uniqueId);
    char* name = (char*) malloc(length + 1);
    sprintf(name, "$%d", *uniqueId);
    (*uniqueId)++;
    return name;
}

/**
 * Each of the following functions takes a token, and returns a list of tokens
 * that represent it using jumps.
 *
 * Also, any generated name should be one more than the highest generated name
 * that appears previous to the generated name's first appearance. In other
 * words, the generated names increase by one each time a new one appears.
 */

Token* toJumpsVisitor(Token* token, uint8_t* uniqueId);

/**
 * Converts an IfToken to a list of tokens using jump tokens.
 * For example, the code
 *
 * if a then
 *     b
 * elif c then
 *     d
 * else
 *     e
 * end
 *
 * becomes
 *
 * condJump a, $next1, 0
 * statement b
 * goto $end
 * label $next1
 * condJump c, $next2, 0
 * statement d
 * goto $end
 * label $next2
 * statement e
 * label $end
 *
 * Note that each condition check jumps to the label right before the next
 * condition check (nextLabel) and the end of each block jumps to the label at
 * the end of the output (endLabel).
 */
Token* toJumpsIf(IfToken* token, uint8_t* uniqueId) {
    List* stmts = NULL;
    //computed lazily to preserve the names of label and conditions
    const char* endLabel = NULL;

    for(List* node = token->branches; node != NULL; node = node->tail) {
        IfBranch* branch = (IfBranch*) node->head;

        //jump to the next branch if the above condition is false
        const char* nextLabel = uniqueName(uniqueId);
        CondJumpToken* condJump = createCondJumpToken(
                branch->condition->location,
                toJumpsVisitor(branch->condition, uniqueId),
                newStr(nextLabel), 0);
        stmts = consList(condJump, stmts);

        //if the condition is true, the block is executed
        Token* stmt = toJumpsVisitor((Token*) branch->block, uniqueId);
        stmts = consList(stmt, stmts);

        //jump to after the if statement
        if(endLabel == NULL) {
            endLabel = uniqueName(uniqueId);
        }
        stmts = consList(createAbsJumpToken(newStr(endLabel)), stmts);

        //indicate the next branch to jump to if the condition was false
        stmts = consList(createLabelToken(newStr(nextLabel)), stmts);

        free((void*) nextLabel);
    }

    //output the else branch
    if(token->elseBranch != NULL) {
        Token* stmt = toJumpsVisitor((Token*) token->elseBranch, uniqueId);
        stmts = consList(stmt, stmts);
    }

    if(endLabel == NULL) {
        endLabel = uniqueName(uniqueId);
    }

    //create the label all if branches jump to after execution
    stmts = consList(createLabelToken(newStr(endLabel)), stmts);

    free((void*) endLabel);
    List* ret = reverseList(stmts);
    destroyShallowList(stmts);
    return (Token*) createBlockToken(token->token.location, ret);
}

/**
 * Converts a while token to a list of tokens using jump tokens.
 * For example the code
 *
 * while a do
 *     b
 * end
 *
 * becomes
 *
 * label $startLabel
 * condJump a, $endLabel, 0
 * b
 * $absJump $startLabel
 * label $endLabel
 */
Token* toJumpsWhile(WhileToken* token, uint8_t* uniqueId) {
    List* stmts = NULL;
    const char* startLabel = uniqueName(uniqueId);

    //beginning of the control structure; jumped to at the end of the loop
    stmts = consList(createLabelToken(newStr(startLabel)), stmts);

    //jump to the end if the condition is false
    const char* endLabel = uniqueName(uniqueId);
    CondJumpToken* condJump = createCondJumpToken(
            token->condition->location,
            toJumpsVisitor(token->condition, uniqueId),
            newStr(endLabel), 0);
    stmts = consList(condJump, stmts);

    //output the body
    stmts = consList(toJumpsVisitor((Token*) token->body, uniqueId), stmts);

    //end of the loop
    stmts = consList(createAbsJumpToken(newStr(startLabel)), stmts);
    stmts = consList(createLabelToken(newStr(endLabel)), stmts);

    free((void*) startLabel);
    free((void*) endLabel);

    List* ret = reverseList(stmts);
    destroyShallowList(stmts);
    return (Token*) createBlockToken(token->token.location, ret);
}

/**
 * Converts a function token's contents from using control structures to using
 * jumps.
 */
Token* toJumpsFunc(FuncToken* token, uint8_t* uniqueId) {
    //copy the arguments since each token holds a unique reference
    List* newArgs = NULL;
    List* oldArgs = token->args;
    while(oldArgs != NULL) {
        IdentifierToken* oldId = (IdentifierToken*) oldArgs->head;
        SrcLoc location = oldId->token.location;
        const char* newName = newStr(oldId->value);
        IdentifierToken* newId = createIdentifierToken(location, newName);
        newArgs = consList((void*) newId, newArgs);
        oldArgs = oldArgs->tail;
    }

    SrcLoc location = token->token.location;
    IdentifierToken* name = (IdentifierToken*)
            copyToken((Token*) token->name, (CopyVisitor) toJumpsVisitor, uniqueId);

    List* args = reverseList(newArgs);
    BlockToken* body = (BlockToken*)
            toJumpsVisitor((Token*) token->body, uniqueId);

    FuncToken* ret = createFuncToken(location, name, args, body);

    destroyShallowList(newArgs);
    return (Token*) ret;
}

/**
 * Converts a list of tokens from using control structures to using jumps and
 * labels.
 *
 * The uniqueId is used to create a unique name for each generated label and
 * variable. It is modified each time a new name is created.
 */
Token* toJumpsVisitor(Token* token, uint8_t* uniqueId) {
    switch(token->type) {
    case TOKEN_INT:
    case TOKEN_FLOAT:
    case TOKEN_LITERAL:
    case TOKEN_IDENTIFIER:
    case TOKEN_TUPLE:
    case TOKEN_LIST:
    case TOKEN_OBJECT:
        //this case shouldn't happen, but its here for future-proofing
    case TOKEN_CALL:
    case TOKEN_BINARY_OP:
    case TOKEN_UNARY_OP:
    case TOKEN_ASSIGNMENT:
    case TOKEN_RETURN:
    case TOKEN_BLOCK:
    case TOKEN_LABEL:
    case TOKEN_ABS_JUMP:
    case TOKEN_COND_JUMP:
    case TOKEN_PUSH_BUILTIN:
    case TOKEN_PUSH_INT:
    case TOKEN_OP_CALL:
    case TOKEN_STORE:
    case TOKEN_DUP:
    case TOKEN_PUSH:
    case TOKEN_ROT3:
    case TOKEN_SWAP:
    case TOKEN_BUILTIN:
    case TOKEN_CHECK_NONE:
    case TOKEN_NEW_FUNC:
        return copyToken(token, (CopyVisitor) toJumpsVisitor, uniqueId);
    case TOKEN_IF:
        return toJumpsIf((IfToken*) token, uniqueId);
    case TOKEN_WHILE:
        return toJumpsWhile((WhileToken*) token, uniqueId);
    case TOKEN_FUNC:
        return toJumpsFunc((FuncToken*) token, uniqueId);
    }
    return NULL;
}

/**
 * Converts the module from using control structures to using jumps and labels.
 */
BlockToken* transformControlToJumps(BlockToken* module) {
    uint8_t uniqueId = 0;
    return (BlockToken*) toJumpsVisitor((Token*) module, &uniqueId);
}

Token* flattenBlocksVisitor(Token* token, void* data);

List* flatList(BlockToken* token) {
    List* list = token->children;
    List* flattened = NULL;

    while(list != NULL) {
        Token* stmt = list->head;
        if(stmt->type == TOKEN_BLOCK) {
            List* output = flatList((BlockToken*) stmt);
            flattened = prependReverseList(output, flattened);
            destroyShallowList(output);
        } else {
            flattened = consList(flattenBlocksVisitor(stmt, NULL), flattened);
        }
        list = list->tail;
    }

    List* ret = reverseList(flattened);
    destroyShallowList(flattened);
    return ret;
}

Token* flattenBlocksVisitor(Token* token, void* data) {
    UNUSED(data);
    switch(token->type) {
     case TOKEN_INT:
     case TOKEN_FLOAT:
     case TOKEN_LITERAL:
     case TOKEN_IDENTIFIER:
     case TOKEN_TUPLE:
     case TOKEN_LIST:
     case TOKEN_OBJECT:
         //this case shouldn't happen, but its here for future-proofing
     case TOKEN_CALL:
     case TOKEN_BINARY_OP:
     case TOKEN_UNARY_OP:
     case TOKEN_ASSIGNMENT:
     case TOKEN_RETURN:
     case TOKEN_IF:
     case TOKEN_WHILE:
     case TOKEN_FUNC:
     case TOKEN_LABEL:
     case TOKEN_ABS_JUMP:
     case TOKEN_COND_JUMP:
         //shouldn't happen anyway
     case TOKEN_PUSH_BUILTIN:
     case TOKEN_PUSH_INT:
     case TOKEN_OP_CALL:
     case TOKEN_STORE:
     case TOKEN_DUP:
     case TOKEN_PUSH:
     case TOKEN_ROT3:
     case TOKEN_SWAP:
     case TOKEN_BUILTIN:
     case TOKEN_CHECK_NONE:
     case TOKEN_NEW_FUNC:
         return copyToken(token, (CopyVisitor) flattenBlocksVisitor, NULL);
     case TOKEN_BLOCK:
         return (Token*) createBlockToken(token->location,
                 flatList((BlockToken*) token));
     }
     return NULL;
}

BlockToken* transformFlattenBlocks(BlockToken* module) {
    return (BlockToken*) flattenBlocksVisitor((Token*) module, NULL);
}

Token* listToConsVisitor(Token* token, void* data);

Token* listToConsListHelper(List* elements) {
    if(elements == NULL) {
        //TODO create a token that always resolves to none, instead of referring
        //to the global scope
        SrcLoc location;
        location.line = 0;
        location.column = 0;
        return (Token*) createBuiltinToken(location, newStr("none"));
    } else {
        const char* op = newStr("::");
        Token* left = listToConsVisitor(elements->head, NULL);
        Token* right = listToConsListHelper(elements->tail);
        return (Token*) createBinaryOpToken(left->location, op, left, right);
    }
}

Token* listToConsList(Token* token) {
    ListToken* list = (ListToken*) token;
    return listToConsListHelper(list->elements);
}

Token* listToConsVisitor(Token* token, void* data) {
    UNUSED(data);
    switch(token->type) {
    case TOKEN_INT:
    case TOKEN_FLOAT:
    case TOKEN_LITERAL:
    case TOKEN_IDENTIFIER:
    case TOKEN_TUPLE:
    case TOKEN_OBJECT: //this case shouldn't happen
    case TOKEN_CALL:
    case TOKEN_BINARY_OP:
    case TOKEN_UNARY_OP:
    case TOKEN_ASSIGNMENT:
    case TOKEN_BLOCK:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_FUNC:
    case TOKEN_RETURN:
    case TOKEN_LABEL:
    case TOKEN_ABS_JUMP:
    case TOKEN_COND_JUMP:
    case TOKEN_PUSH_BUILTIN:
    case TOKEN_PUSH_INT:
    case TOKEN_OP_CALL:
    case TOKEN_STORE:
    case TOKEN_DUP:
    case TOKEN_PUSH:
    case TOKEN_ROT3:
    case TOKEN_SWAP:
    case TOKEN_BUILTIN:
    case TOKEN_CHECK_NONE:
    case TOKEN_NEW_FUNC:
        return copyToken(token, listToConsVisitor, NULL);
    case TOKEN_LIST:
        return listToConsList(token);
    }
    return NULL;
}

BlockToken* transformListToCons(BlockToken* token) {
    return (BlockToken*) listToConsVisitor((Token*) token, NULL);
}

Token* objectDesugarVisitor(Token* token, void* data);

Token* objectDesugarObject(Token* token) {
    ObjectToken* object = (ObjectToken*) token;
    List* tuples = NULL;
    List* elements = object->elements;
    SrcLoc loc = token->location;

    while(elements != NULL) {
        ObjectPair* pair = elements->head;
        Token* key = objectDesugarVisitor(pair->key, NULL);
        Token* value = objectDesugarVisitor(pair->value, NULL);
        List* tupleElems = consList(key, consList(value, NULL));
        Token* tuple = (Token*) createTupleToken(loc, tupleElems);
        tuples = consList(tuple, tuples);
        elements = elements->tail;
    }

    ListToken* list = createListToken(loc, reverseList(tuples));
    destroyShallowList(tuples);
    return (Token*) createUnaryOpToken(loc, newStr("object"), (Token*) list);
}

Token* objectDesugarVisitor(Token* token, void* data) {
    UNUSED(data);
    switch(token->type) {
    case TOKEN_INT:
    case TOKEN_FLOAT:
    case TOKEN_LITERAL:
    case TOKEN_IDENTIFIER:
    case TOKEN_TUPLE:
    case TOKEN_LIST:
    case TOKEN_CALL:
    case TOKEN_BINARY_OP:
    case TOKEN_UNARY_OP:
    case TOKEN_ASSIGNMENT:
    case TOKEN_BLOCK:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_FUNC:
    case TOKEN_RETURN:
    case TOKEN_LABEL:
    case TOKEN_ABS_JUMP:
    case TOKEN_COND_JUMP:
    case TOKEN_PUSH_BUILTIN:
    case TOKEN_PUSH_INT:
    case TOKEN_OP_CALL:
    case TOKEN_STORE:
    case TOKEN_DUP:
    case TOKEN_PUSH:
    case TOKEN_ROT3:
    case TOKEN_SWAP:
    case TOKEN_BUILTIN:
    case TOKEN_CHECK_NONE:
    case TOKEN_NEW_FUNC:
        return copyToken(token, objectDesugarVisitor, NULL);
    case TOKEN_OBJECT:
        return objectDesugarObject(token);
    }
    return NULL; //shouldn't happen
}

Token* transformObjectDesugar(Token* module) {
    return objectDesugarVisitor(module, NULL);
}

Token* destructureVisitor(Token* token, void* data);

Token* destructureLValue(Token* lvalue) {
    SrcLoc loc = lvalue->location;
    if(lvalue->type == TOKEN_IDENTIFIER) {
        IdentifierToken* identifier = (IdentifierToken*) lvalue;
        StoreToken* ret = createStoreToken(loc, newStr(identifier->value));
        return (Token*) ret;
    } else if(lvalue->type == TOKEN_BUILTIN) {
        BuiltinToken* builtin = (BuiltinToken*) lvalue;
        if(strcmp(builtin->name, "none") != 0) {
            //TODO handle error condition
            return NULL;
        }
        return (Token*) createCheckNoneToken(loc);
    } else if(lvalue->type == TOKEN_TUPLE) {
        List* stmts = NULL;
        TupleToken* tuple = (TupleToken*) lvalue;
        List* elements = tuple->elements;
        uint8_t i = 0;
        while(elements != NULL) {
            if(elements->tail != NULL) {
                stmts = consList(createDupToken(loc), stmts);
            }
            stmts = consList(createPushBuiltinToken(loc, newStr("get")), stmts);
            stmts = consList(createPushIntToken(loc, i), stmts);
            stmts = consList(createRot3Token(loc), stmts);
            stmts = consList(createCallOpToken(loc, 2), stmts);
            stmts = consList(destructureLValue(elements->head), stmts);
            i++;
            elements = elements->tail;
        }
        Token* ret = (Token*) createBlockToken(loc, reverseList(stmts));
        destroyShallowList(stmts);
        return ret;
    } else if(lvalue->type == TOKEN_BINARY_OP) {
        BinaryOpToken* binOp = (BinaryOpToken*) lvalue;
        if(strcmp(binOp->op, "::") != 0) {
            //TODO handle error condition
            return NULL;
        }

        List* stmts = NULL;
        //the bytecode assumes that unpack_cons returns a tuple of size 2
        stmts = consList(createPushBuiltinToken(loc, newStr("unpack_cons")), stmts);
        stmts = consList(createSwapToken(loc), stmts);
        stmts = consList(createCallOpToken(loc, 1), stmts);

        stmts = consList(createDupToken(loc), stmts);
        stmts = consList(createPushBuiltinToken(loc, newStr("get")), stmts);
        stmts = consList(createPushIntToken(loc, 0), stmts);
        stmts = consList(createRot3Token(loc), stmts);
        stmts = consList(createCallOpToken(loc, 2), stmts);
        stmts = consList(destructureLValue(binOp->left), stmts);

        stmts = consList(createPushBuiltinToken(loc, newStr("get")), stmts);
        stmts = consList(createPushIntToken(loc, 1), stmts);
        stmts = consList(createRot3Token(loc), stmts);
        stmts = consList(createCallOpToken(loc, 2), stmts);
        stmts = consList(destructureLValue(binOp->right), stmts);

        Token* ret = (Token*) createBlockToken(loc, reverseList(stmts));
        destroyShallowList(stmts);
        return ret;
    } else if(lvalue->type == TOKEN_OBJECT) {
        ObjectToken* object = (ObjectToken*) lvalue;
        List* stmts = NULL;

        List* pairs = object->elements;

        while(pairs != NULL) {
            ObjectPair* pair = pairs->head;

            if(pairs->tail != NULL) {
                stmts = consList(createDupToken(loc), stmts);
            }

            Token* key = copyToken(pair->key, destructureVisitor, NULL);

            stmts = consList(createPushToken(loc, key), stmts);
            stmts = consList(createSwapToken(loc), stmts);
            stmts = consList(createCallOpToken(loc, 1), stmts);
            stmts = consList(destructureLValue(pair->value), stmts);

            pairs = pairs->tail;
        }

        Token* ret = (Token*) createBlockToken(loc, reverseList(stmts));
        destroyShallowList(stmts);
        return ret;
    } else {
        //not implemented yet
        return NULL;
    }
}

Token* destructureAssignment(Token* token) {
    AssignmentToken* assignment = (AssignmentToken*) token;
    List* stmts = NULL;
    Token* right = copyToken(assignment->right, destructureVisitor, NULL);
    stmts = consList(createPushToken(assignment->right->location, right), stmts);
    stmts = consList(destructureLValue(assignment->left), stmts);
    Token* ret = (Token*) createBlockToken(token->location, reverseList(stmts));
    destroyShallowList(stmts);
    return ret;
}

Token* destructureVisitor(Token* token, void* data) {
    UNUSED(data);
    switch(token->type) {
    case TOKEN_INT:
    case TOKEN_FLOAT:
    case TOKEN_LITERAL:
    case TOKEN_IDENTIFIER:
    case TOKEN_TUPLE:
    case TOKEN_LIST:
    case TOKEN_OBJECT:
    case TOKEN_CALL:
    case TOKEN_BINARY_OP:
    case TOKEN_UNARY_OP:
    case TOKEN_BLOCK:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_FUNC:
    case TOKEN_RETURN:
    case TOKEN_LABEL:
    case TOKEN_ABS_JUMP:
    case TOKEN_COND_JUMP:
    case TOKEN_PUSH_BUILTIN:
    case TOKEN_PUSH_INT:
    case TOKEN_OP_CALL:
    case TOKEN_STORE:
    case TOKEN_DUP:
    case TOKEN_PUSH:
    case TOKEN_ROT3:
    case TOKEN_SWAP:
    case TOKEN_BUILTIN:
    case TOKEN_CHECK_NONE:
    case TOKEN_NEW_FUNC:
        return copyToken(token, destructureVisitor, NULL);
    case TOKEN_ASSIGNMENT:
        return destructureAssignment(token);
    }
    return NULL; //shouldn't happen
}

Token* transformDestructure(Token* module) {
    return destructureVisitor(module, NULL);
}

Token* copyVisitor(Token* token, void* data) {
    UNUSED(data);
    return copyToken(token, copyVisitor, NULL);
}

//this is a temporary transformation
Token* transformFuncAssignToName(Token* module) {
    BlockToken* block = (BlockToken*) module;
    List* newStmts = NULL;
    List* oldStmts = block->children;

    while(oldStmts != NULL) {
        Token* stmt = oldStmts->head;
        Token* copy;
        if(stmt->type == TOKEN_ASSIGNMENT) {
            AssignmentToken* assignment = (AssignmentToken*) stmt;
            Token* left = assignment->left;
            Token* right = assignment->right;
            if(left->type == TOKEN_IDENTIFIER && right->type == TOKEN_FUNC) {
                FuncToken* func = (FuncToken*) copyVisitor(right, NULL);
                destroyToken((Token*) func->name);
                func->name = (IdentifierToken*) copyVisitor(left, NULL);
                copy = (Token*) func;
            } else {
                //this is a problem
                copy = copyVisitor(stmt, NULL);
            }
        } else {
            copy = copyVisitor(stmt, NULL);
        }
        newStmts = consList(copy, newStmts);
        oldStmts = oldStmts->tail;
    }

    Token* ret = (Token*) createBlockToken(module->location, reverseList(newStmts));
    destroyShallowList(newStmts);
    return ret;
}

typedef struct {
    List* funcs;
    uint8_t uniqueId;
} ClosureData;

Token* closureVisitor(Token* token, ClosureData* data);

Token* closureCreateFunc(Token* token, ClosureData* data) {
    UNUSED(closureVisitor);
    const char* name = uniqueName(&data->uniqueId);
    FuncToken* func = (FuncToken*)
            copyToken(token, (CopyVisitor) closureVisitor, data);
    destroyToken((Token*) func->name);
    func->name = createIdentifierToken(token->location, name);
    data->funcs = consList(func, data->funcs);

    return (Token*) createPushToken(token->location,
            (Token*) createNewFuncToken(token->location, newStr(name)));
}

Token* closureVisitor(Token* token, ClosureData* data) {
    switch(token->type) {
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_LITERAL:
        case TOKEN_IDENTIFIER:
        case TOKEN_TUPLE:
        case TOKEN_LIST:
        case TOKEN_OBJECT:
        case TOKEN_CALL:
        case TOKEN_BINARY_OP:
        case TOKEN_UNARY_OP:
        case TOKEN_BLOCK:
        case TOKEN_ASSIGNMENT:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_RETURN:
        case TOKEN_LABEL:
        case TOKEN_ABS_JUMP:
        case TOKEN_COND_JUMP:
        case TOKEN_PUSH_BUILTIN:
        case TOKEN_PUSH_INT:
        case TOKEN_OP_CALL:
        case TOKEN_STORE:
        case TOKEN_DUP:
        case TOKEN_PUSH:
        case TOKEN_ROT3:
        case TOKEN_SWAP:
        case TOKEN_BUILTIN:
        case TOKEN_CHECK_NONE:
        case TOKEN_NEW_FUNC:
            return copyToken(token, (CopyVisitor) closureVisitor, data);
        case TOKEN_FUNC:
            return closureCreateFunc(token, data);
        }
        return NULL; //shouldn't happen
}

Token* transformClosures(Token* module) {
    ClosureData data;
    data.funcs = NULL;
    data.uniqueId = 0;

    Token* outside = closureVisitor(module, &data);
    Token* extracted = (Token*) createBlockToken(module->location, data.funcs);

    return (Token*) createBlockToken(module->location,
            consList(outside, consList(extracted, NULL)));
}

Token* transformInitFunc(Token* module) {
    BlockToken* block = (BlockToken*) module;
    List* stmts = block->children;
    List* funcs = NULL;
    List* other = NULL;

    while(stmts != NULL) {
        Token* stmt = copyToken(stmts->head, copyVisitor, NULL);

        if(stmt->type == TOKEN_FUNC) {
            funcs = consList(stmt, funcs);
        } else {
            other = consList(stmt, other);
        }

        stmts = stmts->tail;
    }

    List* oldFuncs = funcs;
    List* oldOther = other;

    funcs = reverseList(funcs);
    other = reverseList(other);

    destroyShallowList(oldFuncs);
    destroyShallowList(oldOther);

    List* parts = NULL;
    parts = consList(createBlockToken(module->location, funcs), NULL);

    IdentifierToken* name = createIdentifierToken(module->location,
            newStr("$init"));
    BlockToken* body = createBlockToken(module->location, other);
    List* args = consList(createIdentifierToken(module->location,
            newStr("$arg")), NULL);
    parts = consList(createFuncToken(module->location, name, args, body), parts);

    return (Token*) createBlockToken(module->location, parts);
}

BlockToken* transformModule(BlockToken* module1) {
    Token* moduleT = transformClosures((Token*) module1);

    BlockToken* module2 = (BlockToken*) transformListToCons((BlockToken*) moduleT);
    destroyToken(moduleT);

    BlockToken* module3 = transformControlToJumps(module2);
    destroyToken((Token*) module2);

    Token* module4 = transformDestructure((Token*) module3);
    destroyToken((Token*) module3);

    Token* module5 = transformObjectDesugar((Token*) module4);
    destroyToken(module4);

    BlockToken* module6 = (BlockToken*) transformListToCons((BlockToken*) module5);
    destroyToken((Token*) module5);

    BlockToken* module7 = transformFlattenBlocks((BlockToken*) module6);
    destroyToken((Token*) module6);

    Token* module8 = transformInitFunc((Token*) module7);
    destroyToken((Token*) module7);

    Token* module9 = (Token*) transformFlattenBlocks((BlockToken*) module8);
    destroyToken(module8);

    return (BlockToken*) module9;
}
