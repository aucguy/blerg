#include "tests.h"

#ifndef PARSETEST_H_
#define PARSETEST_H_

const char* testParseInt();
const char* testParseLiteral();
const char* testParseIdentifier();
const char* testParseExpression();
const char* testParseFail();
const char* testParseAssignments();
const char* parseTestBlockWithEndFails();
const char* parseTestIfStmt();
const char* parseTestWhileStmt();
const char* parseTestFunc();
const char* parseTestFuncWithoutDo();
const char* parseTestGreaterAndLessThan();
const char* parseTestFloat();
const char* parseTestTuple();
const char* parseTestCons();

#endif /* PARSETEST_H_ */
