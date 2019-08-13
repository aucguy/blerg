#include <stdio.h>
#include <string.h>
#include "main/flags.h"

#if INCLUDE_TESTS
#include "test/tests.h"
#endif

int main(int argc, const char* args[]) {
#if INCLUDE_TESTS
    if(argc >= 2 && strcmp(args[1], "--test") == 0) {
        return runTests();
    }
#endif
    return 0;
}
