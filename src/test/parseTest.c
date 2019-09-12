#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test/parseTest.h"
#include "main/parse.h"
#include "main/util.h"

void parseCleanup(ParseState* state, Token* token) {
    free(state);
    destroyToken(token);
}

const char* testParseInt() {
    ParseState* state = createParseState("1234");
    IntToken* token = parseInt(state);

    assert(token->token.type == TOKEN_INT, "token type is not int");
    assert(token->value == 1234, "token value is not 1234");

    parseCleanup(state, (Token*) token);
    return NULL;
}

const char* testParseLiteral() {
    ParseState* state = createParseState("'Hello World'");
    LiteralToken* token = parseLiteral(state);

    assert(token->token.type == TOKEN_LITERAL, "token type is not literal");
    assert(strcmp(token->value, "Hello World") == 0, "token value is not 'Hello World'");

    parseCleanup(state, (Token*) token);
    return NULL;
}

const char* testParseIdentifier() {
    ParseState* state = createParseState("var");
    IdentifierToken* token = parseIdentifier(state);

    assert(token->token.type == TOKEN_IDENTIFIER, "token type is not identifier");
    assert(strcmp(token->value, "var") == 0, "token value is not 'Hello'");

    parseCleanup(state, (Token*) token);
    return NULL;
}

const char* testParseExpression() {
    ParseState* state = createParseState("2 * ( a + 1 ) > 5 and f b == c or not d");
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
                            (Token*) createBinaryOpToken(newStr(" "),
                                    (Token*) createIdentifierToken(newStr("f")),
                                    (Token*) createIdentifierToken(newStr("b"))),
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
    BlockToken* parsed = parseModule("a = 1 + 2; b = 3; c;");

    AssignmentToken* stmt1 = createAssignmentToken(
            createIdentifierToken(newStr("a")),
            (Token*) createBinaryOpToken(newStr("+"),
                    (Token*) createIntToken(1),
                    (Token*) createIntToken(2)));

    AssignmentToken* stmt2 = createAssignmentToken(
            createIdentifierToken(newStr("b")),
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
    BlockToken* parsed = parseModule("a = 1 + 2; b = 3; c; end");
    assert(parsed == NULL, "parse succeeded");
    return NULL;
}

const char* parseTestIfStmt() {
    BlockToken* parsed = parseModule("if a > b ; then c = 1; elif d ; then c = 2; else c = 3; end");

    IfBranch* branch1 = createIfBranch(
            createBlockToken(consList(
                createBinaryOpToken(newStr(">"),
                        (Token*) createIdentifierToken(newStr("a")),
                        (Token*) createIdentifierToken(newStr("b"))), NULL)),
            createBlockToken(consList(
                    createAssignmentToken(
                            createIdentifierToken(newStr("c")),
                            (Token*) createIntToken(1)), NULL)));


    IfBranch* branch2 = createIfBranch(
            createBlockToken(consList(
                (Token*) createIdentifierToken(newStr("d")), NULL)),
            createBlockToken(consList(
                createAssignmentToken(
                        createIdentifierToken(newStr("c")),
                        (Token*) createIntToken(2)), NULL)));

    BlockToken* elseBranch = createBlockToken(consList(
            createAssignmentToken(
                    createIdentifierToken(newStr("c")),
                    (Token*) createIntToken(3)), NULL));

    IfToken* ifToken = createIfToken(consList(branch1, consList(branch2, NULL)), elseBranch);
    BlockToken* expected = createBlockToken(consList(ifToken, NULL));

    assert(tokensEqual((Token*) parsed, (Token*) expected), "incorrect parse");

    destroyToken((Token*) parsed);
    destroyToken((Token*) expected);
    return NULL;
}

const char* parseTestWhileStmt() {
    Token* parsed = (Token*) parseModule("x = 0; while x < 10; do x = x + 1; end");

    Token* stmt1 = (Token*) createAssignmentToken(
            createIdentifierToken(newStr("x")),
            (Token*) createIntToken(0));

    Token* stmt2 = (Token*) createWhileToken(
            createBlockToken(consList(
                    createBinaryOpToken(newStr("<"),
                            (Token*) createIdentifierToken(newStr("x")),
                            (Token*) createIntToken(10)), NULL)),
            createBlockToken(consList(
                    createAssignmentToken(
                            createIdentifierToken(newStr("x")),
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
    Token* parsed = (Token*) parseModule("def add a b do <- a + b; end");

    List* args = consList(createIdentifierToken(newStr("a")),
            consList(createIdentifierToken(newStr("b")), NULL));

    ReturnToken* stmt = createReturnToken(
            (Token*) createBinaryOpToken(newStr("+"),
                    (Token*) createIdentifierToken(newStr("a")),
                    (Token*) createIdentifierToken(newStr("b"))));

    FuncToken* func = createFuncToken(createIdentifierToken(newStr("add")), args,
            createBlockToken(consList(stmt, NULL)));

    Token* expected = (Token*) createBlockToken(consList(func, NULL));

    assert(tokensEqual(parsed, expected), "incorrect parse");

    destroyToken(parsed);
    destroyToken(expected);

    return NULL;
}
