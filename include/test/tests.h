#include <stdio.h>

#ifndef TESTS_H_
#define TESTS_H_

int runTests();

#define assert(cond, msg) if(!(cond)) { return msg; }

#endif /* TESTS_H_ */
