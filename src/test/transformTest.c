#include <stdlib.h>
#include <stdint.h>

#include "main/parse.h"
#include "main/tokens.h"
#include "main/transform.h"
#include "main/util.h"

#include "test/transformTest.h"
#include "test/tests.h"

const char* transformTestControlToJumps() {
    char* error = NULL;
    Token* parsed = (Token*) parseModule("def f z do a = 1; while 0 do if 0 then b = g 2; else c = 3; end end d = 4; end", &error);
    Token* transformed = (Token*) transformControlToJumps((BlockToken*) parsed);

    List* args = consList(createIdentifierToken(newStr("z")), NULL);

    Token* stmts[12] = {
        (Token*) createAssignmentToken(createIdentifierToken(newStr("a")), (Token*) createIntToken(1)),
        (Token*) createLabelToken(newStr("$0")),
        (Token*) createCondJumpToken((Token*) createIntToken(0), newStr("$1"), 0),
        (Token*) createCondJumpToken((Token*) createIntToken(0), newStr("$2"), 0),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("b")), (Token*) createCallToken(
                consList(createIdentifierToken(newStr("g")),
                consList((Token*) createIntToken(2), NULL)))),
        (Token*) createAbsJumpToken(newStr("$3")),
        (Token*) createLabelToken(newStr("$2")),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("c")), (Token*) createIntToken(3)),
        (Token*) createLabelToken(newStr("$3")),
        (Token*) createAbsJumpToken(newStr("$0")),
        (Token*) createLabelToken(newStr("$1")),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("d")), (Token*) createIntToken(4))
    };

    List* body = NULL;
    for(uint8_t i = 12; i != 0; i--) {
        body = consList(stmts[i - 1], body);
    }

    Token* expected = (Token*) createBlockToken(consList(createFuncToken(
            createIdentifierToken(newStr("f")), args, createBlockToken(body)), NULL));

    assert(tokensEqual(expected, transformed), "transformation failed");

    destroyToken(expected);
    destroyToken(parsed);
    destroyToken(transformed);

    return NULL;
}
