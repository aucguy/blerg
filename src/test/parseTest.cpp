#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test/parseTest.h"
#include "main/parse.h"
#include "main/util.h"

#include "test/defaultTokenLoc.h"

void parseCleanup(ParseState* state, Token* token) {
    free(state);
    destroyToken(token);
}

const char* testParseInt() {
    ParseState* state = createParseState("1234");
    IntToken* token = (IntToken*) parseIntOrFloat(state);

    //assert(token->token.type == TOKEN_INT, "token type is not int");
    assert(getTokenType((Token*) token) == TOKEN_INT, "token type is not int");
    assert(token->value == 1234, "token value is not 1234");

    parseCleanup(state, (Token*) token);
    return NULL;
}

const char* testParseLiteral() {
    ParseState* state = createParseState("'Hello World'");
    LiteralToken* token = parseLiteral(state);

    assert(getTokenType((Token*) token) == TOKEN_LITERAL, "token type is not literal");
    assert(strcmp(token->value, "Hello World") == 0, "token value is not 'Hello World'");

    parseCleanup(state, (Token*) token);
    return NULL;
}

const char* testParseIdentifier() {
    ParseState* state = createParseState("var");
    IdentifierToken* token = parseIdentifier(state);

    assert(getTokenType((Token*) token) == TOKEN_IDENTIFIER, "token type is not identifier");
    assert(strcmp(token->value, "var") == 0, "token value is not 'Hello'");

    parseCleanup(state, (Token*) token);
    return NULL;
}

const char* testParseExpression() {
    ParseState* state = createParseState("2 * ( a + 1 ) > 5 and f b z == c or not d");
    Token* parsed = parseExpression(state);

    BinaryOpToken* expected = createBinaryOpToken(newStr("or"),
            (Token*) createBinaryOpToken(newStr("and"),
                    (Token*) createBinaryOpToken(newStr(">"),
                            (Token*) createBinaryOpToken(newStr("*"),
                                    (Token*) createIntToken(2),
                                    (Token*) createBinaryOpToken(newStr("+"),
                                            (Token*) createIdentifierToken(newStr("a")),
                                            (Token*) createIntToken(1))),
                            (Token*) createIntToken(5)),
                    (Token*) createBinaryOpToken(newStr("=="),
                    		(Token*) createCallToken(
                    		        consList(createIdentifierToken(newStr("f")),
                    				consList(createIdentifierToken(newStr("b")),
                    				consList(createIdentifierToken(newStr("z")), NULL)))),
                            (Token*) createIdentifierToken(newStr("c")))),
            (Token*) createUnaryOpToken(newStr("not"),
                    (Token*) createIdentifierToken(newStr("d"))));

    assert(tokensEqual(parsed, (Token*) expected), "incorrect parse");

    free(state);
    destroyToken(parsed);
    destroyToken((Token*) expected);

    return NULL;
}

const char* testParseFail() {
    ParseState* state = createParseState("'hello");
    assert(parseExpression(state) == NULL, "parse succeeded for identifier");
    free(state);

    state = createParseState("1 + (a *");
    assert(parseExpression(state) == NULL, "parse succeeded for parenthesis");
    free(state);

    state = createParseState("2 * %");
    assert(parseExpression(state) == NULL, "parse succeeded, for unknown factor");
    free(state);
    return NULL;
}

const char* testParseAssignments() {
    char* error = NULL;
    BlockToken* parsed = parseModule("a = 1 + 2; b = 3; c;", &error);

    AssignmentToken* stmt1 = createAssignmentToken(
            (Token*) createIdentifierToken(newStr("a")),
            (Token*) createBinaryOpToken(newStr("+"),
                    (Token*) createIntToken(1),
                    (Token*) createIntToken(2)));

    AssignmentToken* stmt2 = createAssignmentToken(
            (Token*) createIdentifierToken(newStr("b")),
            (Token*) createIntToken(3));

    IdentifierToken* stmt3 = createIdentifierToken(newStr("c"));

    List* list = consList(stmt1, consList(stmt2, consList(stmt3, NULL)));
    BlockToken* expected = createBlockToken(list);

    assert(tokensEqual((Token*) parsed, (Token*) expected), "incorrect parse");

    destroyToken((Token*) parsed);
    destroyToken((Token*) expected);
    return NULL;
}

const char* parseTestBlockWithEndFails() {
    char* error = NULL;
    BlockToken* parsed = parseModule("a = 1 + 2; b = 3; c; end", &error);
    assert(parsed == NULL, "parse succeeded");
    if(error != NULL) {
        free(error);
    }
    return NULL;
}

const char* parseTestIfStmt() {
    char* error = NULL;
    BlockToken* parsed = parseModule("if a > b then c = 1; elif d then c = 2; else c = 3; end", &error);

    IfBranch* branch1 = createIfBranch(
            (Token*) createBinaryOpToken(newStr(">"),
                    (Token*) createIdentifierToken(newStr("a")),
                    (Token*) createIdentifierToken(newStr("b"))),
            createBlockToken(consList(
                    createAssignmentToken(
                            (Token*) createIdentifierToken(newStr("c")),
                            (Token*) createIntToken(1)), NULL)));


    IfBranch* branch2 = createIfBranch(
            (Token*) createIdentifierToken(newStr("d")),
            createBlockToken(consList(
                createAssignmentToken(
                        (Token*) createIdentifierToken(newStr("c")),
                        (Token*) createIntToken(2)), NULL)));

    BlockToken* elseBranch = createBlockToken(consList(
            createAssignmentToken(
                    (Token*) createIdentifierToken(newStr("c")),
                    (Token*) createIntToken(3)), NULL));

    IfToken* ifToken = createIfToken(consList(branch1, consList(branch2, NULL)), elseBranch);
    BlockToken* expected = createBlockToken(consList(ifToken, NULL));

    assert(tokensEqual((Token*) parsed, (Token*) expected), "incorrect parse");

    destroyToken((Token*) parsed);
    destroyToken((Token*) expected);
    return NULL;
}

const char* parseTestWhileStmt() {
    char* error = NULL;
    Token* parsed = (Token*) parseModule("x = 0; while x < 10 do x = x + 1; end", &error);

    Token* stmt1 = (Token*) createAssignmentToken(
            (Token*) createIdentifierToken(newStr("x")),
            (Token*) createIntToken(0));

    Token* stmt2 = (Token*) createWhileToken(
            (Token*) createBinaryOpToken(newStr("<"),
                    (Token*) createIdentifierToken(newStr("x")),
                    (Token*) createIntToken(10)),
            createBlockToken(consList(
                    createAssignmentToken(
                            (Token*) createIdentifierToken(newStr("x")),
                            (Token*) createBinaryOpToken(newStr("+"),
                                    (Token*) createIdentifierToken(newStr("x")),
                                    (Token*) createIntToken(1))), NULL)));

    Token* expected = (Token*) createBlockToken(consList(stmt1, consList(stmt2, NULL)));
    assert(tokensEqual(parsed, expected), "incorrect parse");

    destroyToken(parsed);
    destroyToken(expected);
    return NULL;
}

const char* parseTestFunc() {
    char* error = NULL;
    Token* parsed = (Token*) parseModule("def a b do return a + b; end;", &error);

    List* args = consList(createIdentifierToken(newStr("a")),
            consList(createIdentifierToken(newStr("b")), NULL));

    ReturnToken* stmt = createReturnToken(
            (Token*) createBinaryOpToken(newStr("+"),
                    (Token*) createIdentifierToken(newStr("a")),
                    (Token*) createIdentifierToken(newStr("b"))));

    FuncToken* func = createFuncToken(createIdentifierToken(newStr("name")), args,
            createBlockToken(consList(stmt, NULL)));

    Token* expected = (Token*) createBlockToken(consList(func, NULL));

    assert(tokensEqual(parsed, expected), "incorrect parse");

    destroyToken(parsed);
    destroyToken(expected);

    return NULL;
}

const char* parseTestFuncWithoutDo() {
    char* error = NULL;
    parseModule("def one return 1; end", &error);
    if(error != NULL) {
        free(error);
    }
    return NULL;
}

const char* parseTestGreaterAndLessThan() {
    char* error = NULL;
    Token* parsed = (Token*) parseModule("def x do return 1 <= 2 and 3 >= 4; end;", &error);

    Token* ret = (Token*) createReturnToken(
            (Token*) createBinaryOpToken(newStr("and"),
                    (Token*) createBinaryOpToken(newStr("<="),
                            (Token*) createIntToken(1),
                            (Token*) createIntToken(2)),
                    (Token*) createBinaryOpToken(newStr(">="),
                            (Token*) createIntToken(3),
                            (Token*) createIntToken(4))));

    Token* expected = (Token*) createBlockToken(consList(
            createFuncToken(
                    createIdentifierToken(newStr("name")),
                    consList(createIdentifierToken(newStr("x")), NULL),
                    createBlockToken(consList(ret, NULL))), NULL));


    assert(tokensEqual(parsed, expected), "incorrect parse");
    destroyToken(parsed);
    destroyToken(expected);
    return NULL;
}

const char* parseTestFloat() {
    ParseState* state = createParseState("-3.14e5");
    Token* parsed = (Token*) parseIntOrFloat(state);
    Token* expected = (Token*) createFloatToken(-3.14e5);

    assert(tokensEqual(parsed, expected), "incorrect parse");

    destroyToken(parsed);
    destroyToken(expected);
    free(state);
    return NULL;
}

const char* parseTestTuple() {
    ParseState* state = createParseState("(1, 2, 3)");
    Token* parsed = parseFactor(state);

    List* elements = consList(createIntToken(3), NULL);
    elements = consList(createIntToken(2), elements);
    elements = consList(createIntToken(1), elements);
    Token* expected = (Token*) createTupleToken(elements);

    assert(tokensEqual(parsed, expected), "incorrect parse");

    destroyToken(parsed);
    destroyToken(expected);
    free(state);
    return NULL;
}

const char* parseTestCons() {
    ParseState* state = createParseState("1 :: 2");
    Token* parsed = parseExpression(state);

    Token* expected = (Token*) createBinaryOpToken(newStr("::"),
            (Token*) createIntToken(1),
            (Token*) createIntToken(2));

    assert(tokensEqual(parsed, expected), "incorrect parse");

    destroyToken(parsed);
    destroyToken(expected);
    free(state);
    return NULL;
}

const char* parseTestObject() {
    //this code would fail in execution, but is syntactically valid
    ParseState* state = createParseState("{ property: 'value', 1 + 2: 3 * 4 }");
    Token* parsed = parseFactor(state);

    Token* key = (Token*) createIdentifierToken(newStr("property"));
    Token* value = (Token*) createLiteralToken(newStr("value"));
    ObjectPair* pair1 = createObjectPair(key, value);

    key = (Token*) createBinaryOpToken(newStr("+"),
            (Token*) createIntToken(1),
            (Token*) createIntToken(2));

    value = (Token*) createBinaryOpToken(newStr("*"),
            (Token*) createIntToken(3),
            (Token*) createIntToken(4));

    ObjectPair* pair2 = createObjectPair(key, value);

    Token* expected = (Token*) createObjectToken(
            consList(pair1, consList(pair2, NULL)));

    assert(tokensEqual(parsed, expected), "incorrect parse");

    destroyToken(parsed);
    destroyToken(expected);
    free(state);
    return NULL;
}
