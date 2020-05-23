#ifndef TOP_H_
#define TOP_H_

#include <stdint.h>

#include "main/flags.h"
#include "main/runtime.h"
#include "main/execute.h"
#include "main/thing.h"

typedef struct {
    Runtime* runtime;
    const char* src;
    const char* name;
    Thing** args;
    uint8_t arity;
} ExecFuncIn;

typedef struct {
    RetVal retVal;
    const char* errorMsg;
    Module* module;
} ExecFuncOut;

char* readFile(const char* filename);

Module* sourceToModule(const char* src, char** error);

ExecFuncOut execFunc(ExecFuncIn in);
void cleanupExecFunc(ExecFuncIn in, ExecFuncOut out);

#endif /* TOP_H_ */
