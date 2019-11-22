#include <stdlib.h>

#include "main/parse.h"
#include "main/tokens.h"
#include "main/transform.h"

#include "test/transformTest.h"
#include "test/tests.h"

const char* transformTestControlToJumps() {
    Token* parsed = (Token*) parseModule("def f z do a = 1; while 0; do if 0; then b = 2; else c = 3; end end d = 4; end");
    Token* transformed = (Token*) transformControlToJumps((BlockToken*) parsed);

    List* args = consList(createIdentifierToken(newStr("z")), NULL);

    Token* stmts[14] = {
        (Token*) createAssignmentToken(createIdentifierToken(newStr("a")), (Token*) createIntToken(1)),
        (Token*) createLabelToken(newStr("$0")),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("$1")), (Token*) createIntToken(0)),
        (Token*) createCondJumpToken(newStr("$1"), newStr("$2"), 0),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("$3")), (Token*) createIntToken(0)),
        (Token*) createCondJumpToken(newStr("$3"), newStr("$4"), 0),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("b")), (Token*) createIntToken(2)),
        (Token*) createAbsJumpToken(newStr("$5")),
        (Token*) createLabelToken(newStr("$4")),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("c")), (Token*) createIntToken(3)),
        (Token*) createLabelToken(newStr("$5")),
        (Token*) createAbsJumpToken(newStr("$0")),
        (Token*) createLabelToken(newStr("$2")),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("d")), (Token*) createIntToken(4))
    };

    List* body = NULL;
    for(int i = 13; i >= 0; i--) {
        body = consList(stmts[i], body);
    }

    Token* expected = (Token*) createBlockToken(consList(createFuncToken(
            createIdentifierToken(newStr("f")), args, createBlockToken(body)), NULL));

    assert(tokensEqual(expected, transformed), "transformation failed");

    destroyToken(expected);
    destroyToken(parsed);
    destroyToken(transformed);

    return NULL;
}
