#include "tests.h"

#ifndef PARSETEST_H_
#define PARSETEST_H_

const char* testParseInt();
const char* testParseLiteral();
const char* testParseIdentifier();
const char* testParseExpression();
const char* testParseFail();
const char* testParseAssignments();
const char* parseTestBlockWithoutEndFails();
const char* parseTestIfStmt();
const char* parseTestWhileStmt();

#endif /* PARSETEST_H_ */
