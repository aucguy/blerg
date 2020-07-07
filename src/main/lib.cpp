#include <main/thing.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "main/execute.h"
#include "main/top.h"
#include "main/std_lib/modules.h"

#define UNUSED(x) (void)(x)

RetVal libPrint(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_STR);
    if(isRetValError(retVal)) {
        return retVal;
    }

    printf("%s\n", thingAsStr(args[0]));
    return createRetVal(runtime->noneThing, 0);
}

//TODO accept arbitrary long line lengths and remove excess data
RetVal libInput(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    UNUSED(args);
    UNUSED(arity);

    char* str = (char*) malloc(sizeof(char) * 100);
    fgets(str, 100, stdin);
    //remove newline
    str[strlen(str) - 1] = 0;
    return createRetVal(createStrThing(runtime, str, 0), 0);
}

RetVal libAssert(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    UNUSED(arity);

    RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_BOOL);
    if(isRetValError(retVal)) {
        return retVal;
    }

    if(thingAsBool(args[0]) == 0) {
        return throwMsg(runtime, newStr("assertion failure: argument is false"));
    }

    return createRetVal(runtime->noneThing, 0);
}

RetVal libToStr(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_INT);
    if(isRetValError(retVal)) {
        return retVal;
    }

    //TODO don't make fixed size
    char* str = (char*) malloc(sizeof(char) * 100);
    sprintf(str, "%i", thingAsInt(args[0]));
    return createRetVal(createStrThing(runtime, str, 0), 0);
}

RetVal libToInt(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_STR);
    if(isRetValError(retVal)) {
        return retVal;
    }
    int32_t i = strtol(thingAsStr(args[0]), NULL, 10);
    return createRetVal(createIntThing(runtime, i), 0);
}

RetVal libTryCatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    if(arity != 2) {
        return throwMsg(runtime, formatStr("expected 2 arguments but got %i"));
    }

    Thing* block1 = args[0];
    Thing* block2 = args[1];

    Thing** passedArgs = (Thing**) malloc(sizeof(Thing*) * 1);
    passedArgs[0] = runtime->noneThing;

    RetVal result = callFunction(runtime, block1, 1, passedArgs);

    if(!isRetValError(result)) {
        free(passedArgs);
        return result;
    }

    passedArgs[0] = getRetVal(result);
    result = callFunction(runtime, block2, 1, passedArgs);
    free(passedArgs);
    return result;
}

RetVal libTuple(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    //a copy must be made since args is freed after the function
    Thing** copy = (Thing**) malloc(sizeof(Thing*) * arity);

    for(uint8_t i = 0 ; i < arity; i++) {
        copy[i] = args[i];
    }

    return createRetVal(createTupleThing(runtime, arity, copy), 0);
}

RetVal libCons(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    if(arity != 2) {
        return throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
    }

    ThingType* type = typeOfThing(args[1]);

    if(type != THING_TYPE_NONE && type != THING_TYPE_LIST) {
        const char* msg = formatStr("expected argument 2 to be none or a list");
        return throwMsg(runtime, msg);
    }

    return createRetVal(createListThing(runtime, args[0], args[1]), 0);
}

RetVal libHead(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    RetVal ret = typeCheck(runtime, self, args, arity, 1, THING_TYPE_LIST);
    if(isRetValError(ret)) {
        return ret;
    }

    return createRetVal(((ListThing*) args[0])->head, 0);
}

RetVal libTail(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    RetVal ret = typeCheck(runtime, self, args, arity, 1, THING_TYPE_LIST);
    if(isRetValError(ret)) {
        return ret;
    }

    return createRetVal(((ListThing*) args[0])->tail, 0);
}

RetVal libCreateSymbol(Runtime* runtime, Thing* self, Thing** args,
        uint8_t arity) {
    RetVal ret = typeCheck(runtime, self, args, arity, 1, THING_TYPE_INT);
    if(isRetValError(ret)) {
        return ret;
    }

    uint8_t id = newSymbolId();
    uint32_t argNo = thingAsInt(args[0]);

    if(argNo == 0) {
        return throwMsg(runtime, newStr("symbol arity cannot be 0"));
    } else if(argNo > 255) {
        const char* msg = "symbol arity cannot be more than 255";
        return throwMsg(runtime, newStr(msg));
    }

    Thing* symbol = createSymbolThing(runtime, id, argNo);
    return createRetVal(symbol, 0);
}

RetVal libObject(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    //TODO allow empty objects
    RetVal ret = typeCheck(runtime, self, args, arity, 1, THING_TYPE_LIST);
    if(isRetValError(ret)) {
        return ret;
    }

    Map* map = createMap();

    ListThing* elements = (ListThing*) args[0];
    while(typeOfThing(elements) == THING_TYPE_LIST) {
        if(typeOfThing(elements->head) != THING_TYPE_TUPLE) {
            const char* msg = "internal error: expected pair to be a tuple";
            return throwMsg(runtime, newStr(msg));
        }

        Thing* pair = elements->head;
        if(getTupleSize(pair) != 2) {
            const char* msg = "internal error: expected pair to have two elements";
            return throwMsg(runtime, newStr(msg));
        }

        Thing* key = getTupleElem(pair, 0);
        if(typeOfThing(key) != THING_TYPE_SYMBOL) {
            const char* msg = "key is not a symbol";
            return throwMsg(runtime, newStr(msg));
        }

        Thing* value = getTupleElem(pair, 1);
        putMapUint32(map, getSymbolId(key), value);

        elements = (ListThing*) elements->tail;
    }

    return createRetVal(createObjectThing(runtime, map), 0);
}

RetVal libUnpackCons(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    //even though none is a list, it can't be unpacked, so we only check for lists
    //TODO give a better error message when one attempts to unpack a none
    RetVal ret = typeCheck(runtime, self, args, arity, 1, THING_TYPE_LIST);
    if(isRetValError(ret)) {
        return ret;
    }

    ListThing* list = (ListThing*) args[0];
    Thing** elements = (Thing**) malloc(2 * sizeof(Thing*));
    elements[0] = list->head;
    elements[1] = list->tail;
    return createRetVal(createTupleThing(runtime, 2, elements), 0);
}

RetVal libCreateCell(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    if(arity != 1) {
        const char* msg = formatStr("expected 1 argument but got %i", arity);
        return throwMsg(runtime, msg);
    }

    return createRetVal(createCellThing(runtime, args[0]), 0);
}

RetVal libGetCell(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    RetVal ret = typeCheck(runtime, self, args, arity, 1, THING_TYPE_CELL);
    if(isRetValError(ret)) {
        return ret;
    }

    return createRetVal(getCellValue(args[0]), 0);
}

RetVal libSetCell(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    if(arity != 2) {
        const char* msg = formatStr("expected 2 arguments but got %i", arity);
        return throwMsg(runtime, msg);
    }

    if(typeOfThing(args[0]) != THING_TYPE_CELL) {
        return throwMsg(runtime, newStr("expected argument 1 to be a cell"));
    }

    setCellValue(args[0], args[1]);
    return createRetVal(runtime->noneThing, 0);
}

uint8_t fileExists(const char* name) {
    struct stat fileStat;
    uint8_t error = stat(name, &fileStat);
    return error == 0 && (fileStat.st_mode & S_IFREG);
}

const char* getModulePath(Runtime* runtime, const char* name) {
    if(fileExists(name)) {
        return newStr(name);
    }
    const char* stdPath = formatStr("%s/std_lib/%s", runtime->execDir, name);
    if(fileExists(stdPath)) {
        return stdPath;
    } else {
        free((char*) stdPath);
        return NULL;
    }
}

//TODO make this work on non-posix systems
RetVal libImport(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    RetVal ret = typeCheck(runtime, self, args, arity, 1, THING_TYPE_STR);
    if(isRetValError(ret)) {
        return ret;
    }

    const char* filename = thingAsStr(args[0]);
    //TODO normalize import path
    Thing* moduleThing = getMapStr(runtime->modules, filename);
    if(moduleThing != NULL) {
        return createRetVal(moduleThing, 0);
    }

    const char* path = getModulePath(runtime, filename);
    if(path != NULL) {
        char* src = readFile(path);
        free((char*) path);
        char* errorMsg = NULL;
        Module* module = sourceToModule(path, src, &errorMsg);
        free(src);
        if(errorMsg != NULL) {
            return throwMsg(runtime, errorMsg);
        }
        runtime->moduleBytecode = consList(module, runtime->moduleBytecode);
        ret = executeModule(runtime, module);
        if(isRetValError(ret)) {
            return ret;
        }
        putMapStr(runtime->modules, filename, getRetVal(ret));
        return ret;
    }

    moduleThing = loadBuiltinModule(runtime, filename);
    if(moduleThing == NULL) {
        const char* format = "could not find module %s";
        return throwMsg(runtime, formatStr(format, filename));
    }

    putMapStr(runtime->modules, filename, moduleThing);
    return createRetVal(moduleThing, 0);
}

RetVal libIsNone(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    if(arity != 1) {
        return throwMsg(runtime, formatStr("expected 1 arg but got %i", arity));
    }

    uint8_t ret = typeOfThing(args[0]) == THING_TYPE_NONE;
    return createRetVal(createBoolThing(runtime, ret), 0);
}

RetVal libUnpackCall(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    if(arity != 3) {
        return throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
    }

    if(typeOfThing(args[2]) != THING_TYPE_INT) {
        return throwMsg(runtime, newStr("expected argument 2 to be an int"));
    }

    Thing* passedArgs[2] = {
            args[0],
            args[1]
    };

    Thing* sym = createSymbolThing(runtime, SYM_UNPACK, 2);
    RetVal ret = callFunction(runtime, sym, 2, passedArgs);

    if(isRetValError(ret)) {
        return ret;
    }

    Thing* tup = getRetVal(ret);

    if(typeOfThing(tup) != THING_TYPE_TUPLE) {
        const char* str = "expected destructured value to be a tuple";
        return throwMsg(runtime, newStr(str));
    }

    if(getTupleSize(tup) != thingAsInt(args[2])) {
        return throwMsg(runtime, newStr("tuple is not the correct size"));
    }

    return createRetVal(tup, 0);
}

RetVal libAssertEqual(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);
    if(arity != 2) {
        return throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
    }

    Thing* symbol = getMapStr(runtime->operators, "==");
    RetVal ret = callFunction(runtime, symbol, 2, args);

    if(isRetValError(ret)) {
        return ret;
    }

    Thing* val = getRetVal(ret);

    if(typeOfThing(val) != THING_TYPE_BOOL) {
        return throwMsg(runtime, newStr("result of == is not a bool"));
    }

    if(!thingAsBool(val)) {
        return throwMsg(runtime, newStr("assertion failed"));
    }

    return createRetVal(runtime->noneThing, 0);
}
