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
