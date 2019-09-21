#ifndef PARSE_H_
#define PARSE_H_

#include "main/flags.h"
#include "main/util.h"
#include "main/tokens.h"

#if INCLUDE_TESTS || IS_PARSE_IMPL
/**
 * Holds the source code and the current parse position in it. The parsing
 * functions increment the index as they consume tokens.
 */
typedef struct {
    const char* src;
    int index;
} ParseState;

/**
 * Creates a ParseState from the given source code.
 */
ParseState* createParseState(const char*);

/**
 * Parses the module
 *
 * @param src the source code for the given module
 * @return the AST for the source
 */
BlockToken* parseModule(const char* src);

#if INCLUDE_TESTS

IntToken* parseInt(ParseState*);
LiteralToken* parseLiteral(ParseState*);
IdentifierToken* parseIdentifier(ParseState*);
Token* parseTerm(ParseState*);
Token* parseExpression(ParseState*);

#endif

#endif

#endif /* PARSE_H_ */
