#include <stdlib.h>
#include "main/tokens.h"

int validateOnlyFuncsToplevel(BlockToken* module) {
    if(module->token.type != TOKEN_BLOCK) {
        return 0;
    }
    List* node = module->children;
    while(node != NULL) {
        if(((Token*) node->head)->type != TOKEN_FUNC) {
            return 0;
        }
        node = node->tail;
    }
    return 1;
}

int containsNoFuncsBranch(void*);
int containsNoFuncsVoid(void*);

/**
 * Returns true if the token and all of its subtokens are not functions.
 */
int containsNoFuncs(Token* token) {
    if(token == NULL) {
        return 1;
    } else if(token->type == TOKEN_BLOCK) {
        return allList(((BlockToken*) token)->children, containsNoFuncsVoid);
    } else if(token->type == TOKEN_IF) {
        IfToken* ifToken = (IfToken*) token;
        return allList(ifToken->branches, containsNoFuncsBranch) &&
                containsNoFuncs((Token*) ifToken->elseBranch);
    } else if(token->type == TOKEN_WHILE) {
        WhileToken* whileToken = (WhileToken*) token;
        return containsNoFuncs((Token*) whileToken->condition) &&
                containsNoFuncs((Token*) whileToken->body);
    } else if(token->type == TOKEN_FUNC) {
        return 0;
    } else {
        return 1;
    }
}

int containsNoFuncsVoid(void* token) {
    return containsNoFuncs((Token*) token);
}

/**
 * Returns true if the branch does not contain functions.
 */
int containsNoFuncsBranch(void* branch) {
    IfBranch* casted = (IfBranch*) branch;
    return containsNoFuncs((Token*) casted->condition) &&
            containsNoFuncs((Token*) casted->block);
}

/**
 * Returns true if the given token does not contain functions. It does not
 * matter if the token itself is not a function. This function only works on
 * function tokens.
 */
int noInnerFuncs(Token* token) {
    if(token->type == TOKEN_FUNC) {
        if(!allList(((FuncToken*) token)->body->children,
                (int (*)(void*)) containsNoFuncs)) {
            return 0;
        }
    }
    return 1;
}

/**
 * Returns true if the module contains no inner functions
 */
int validateNoInnerFuncs(BlockToken* module) {
    return allList(module->children, (int (*)(void*)) noInnerFuncs);
}
