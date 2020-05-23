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

Module* sourceToModule(const char* src, char** error) {
    BlockToken* ast = parseModule(src, error);
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
    out.retVal = createRetVal(NULL, 0);
    out.errorMsg = NULL;
    out.module = NULL;

    out.module = sourceToModule(in.src, &out.errorMsg);
    if(out.module == NULL) {
        return out;
    }

    RetVal global = executeModule(in.runtime, out.module);
    if(isRetValError(global)) {
        out.errorMsg = errorStackTrace(in.runtime, getRetVal(global));
        return out;
    } else if(typeOfThing(getRetVal(global)) != THING_TYPE_OBJ) {
        out.errorMsg = newStr("global scope is not an object");
        return out;
    }

    Thing* func = getObjectProperty(getRetVal(global), in.name);
    if(func == NULL) {
        out.errorMsg = newStr("function not found");
        return out;
    } else if(typeOfThing(func) != THING_TYPE_FUNC) {
        out.errorMsg = newStr("function is not a function");
        return out;
    }

    out.retVal = callFunction(in.runtime, func, in.arity, in.args);
    if(isRetValError(out.retVal)) {
        out.errorMsg = errorStackTrace(in.runtime, getRetVal(out.retVal));
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
    free(out.errorMsg);
    deinitThing();
}
