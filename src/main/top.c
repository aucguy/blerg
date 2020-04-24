#include <stdio.h>
#include <stdlib.h>

#include "main/parse.h"
#include "main/tokens.h"
#include "main/validate.h"
#include "main/transform.h"
#include "main/codegen.h"
#include "main/execute.h"
#include "main/thing.h"
#include "main/top.h"

//from https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
char* readFile(const char* filename) {
    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;
    return string;
}

Module* sourceToModule(const char* src) {
    BlockToken* ast = parseModule(src);
    if(ast == NULL) {
        return NULL;
    }
    if(!validateModule(ast)) {
        return NULL;
    }
    BlockToken* transformed = transformModule(ast);
    destroyToken((Token*) ast);
    Module* module = compileModule((Token*) transformed);
    destroyToken((Token*) transformed);
    return module;
}

ExecFuncOut execFunc(ExecFuncIn in) {
    ExecFuncOut out;
    out.retVal = NULL;
    out.errorMsg = NULL;
    out.module = NULL;

    uint8_t error = 0;
    out.module = sourceToModule(in.src);
    if(out.module == NULL) {
        out.errorMsg = "error in source code";
        return out;
    }

    Thing* global = executeModule(in.runtime, out.module, &error);
    if(error != 0) {
        out.errorMsg = "error executing global scope";
        return out;
    } else if(typeOfThing(global) != THING_TYPE_OBJ) {
        out.errorMsg = "global scope is not an object";
        return out;
    }

    Thing* func = getObjectProperty(global, in.name);
    if(func == NULL) {
        out.errorMsg = "function not found";
        return out;
    } else if(typeOfThing(func) != THING_TYPE_FUNC) {
        out.errorMsg = "function is not a function";
        return out;
    }

    out.retVal = callFunction(in.runtime, func, in.arity, in.args, &error);
    if(error != 0) {
        out.errorMsg = "error executing function";
        return out;
    }
    return out;
}

void cleanupExecFunc(ExecFuncIn in, ExecFuncOut out) {
    if(in.runtime != NULL) {
        destroyRuntime(in.runtime);
    }

    if(out.module != NULL) {
        destroyModule(out.module);
    }

    free(in.args);
    deinitThing();
}
