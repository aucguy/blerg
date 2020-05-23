#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "main/execute.h"
#include "main/thing.h"

#define UNUSED(x) (void)(x)

ThingType* createThingType() {
    ThingType* type = malloc(sizeof(ThingType));
    type->destroy = NULL;
    return type;
}

void setDestroyThingType(ThingType* type, void (*destroy)(Thing*)) {
    type->destroy = destroy;
}

void setCallThingType(ThingType* type, ExecFunc call) {
    type->call = call;
}

void setDispatchThingType(ThingType* type, ExecFunc dispatch) {
    type->dispatch = dispatch;
}

void destroySimpleThing(Thing* thing) {
    UNUSED(thing);
}

RetVal errorCall(Runtime* runtime, Thing* thing, Thing** args, uint8_t arity) {
    UNUSED(runtime);
    UNUSED(thing);
    UNUSED(arity);
    UNUSED(args);
    //TODO report the object type
    return throwMsg(runtime, "cannot call this type");
}

RetVal symbolCall(Runtime*, Thing*, Thing**, uint8_t);
RetVal intDispatch(Runtime*, Thing*, Thing**, uint8_t);
RetVal strDispatch(Runtime*, Thing*, Thing**, uint8_t);
void destroyStrThing(Thing*);
RetVal boolDispatch(Runtime*, Thing*, Thing**, uint8_t);
uint32_t getSymbolId(Thing* self);
RetVal nativeFuncCall(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

void destroyObjectThing(Thing* thing);
void destroyErrorThing(Thing* thing);

static uint8_t initialized = 0;

void initThing() {
    if(!initialized) {
        THING_TYPE_NONE = createThingType();
        setDestroyThingType(THING_TYPE_NONE, destroySimpleThing);
        setCallThingType(THING_TYPE_NONE, errorCall);
        setDispatchThingType(THING_TYPE_NONE, errorCall);

        THING_TYPE_SYMBOL = createThingType();
        setDestroyThingType(THING_TYPE_SYMBOL, destroySimpleThing);
        setCallThingType(THING_TYPE_SYMBOL, symbolCall);
        setDispatchThingType(THING_TYPE_SYMBOL, symbolCall);

        THING_TYPE_INT = createThingType();
        setDestroyThingType(THING_TYPE_INT, destroySimpleThing);
        setCallThingType(THING_TYPE_INT, errorCall);
        setDispatchThingType(THING_TYPE_INT, intDispatch);

        THING_TYPE_STR = createThingType();
        setDestroyThingType(THING_TYPE_STR, destroyStrThing);
        setCallThingType(THING_TYPE_STR, errorCall);
        setDispatchThingType(THING_TYPE_STR, strDispatch);

        THING_TYPE_BOOL = createThingType();
        setDestroyThingType(THING_TYPE_BOOL, destroySimpleThing);
        setCallThingType(THING_TYPE_BOOL, errorCall);
        setDispatchThingType(THING_TYPE_BOOL, boolDispatch);

        THING_TYPE_OBJ = createThingType();
        setDestroyThingType(THING_TYPE_OBJ, destroyObjectThing);
        setCallThingType(THING_TYPE_OBJ, errorCall);
        setDispatchThingType(THING_TYPE_OBJ, errorCall);

        THING_TYPE_FUNC = createThingType();
        setDestroyThingType(THING_TYPE_FUNC, destroySimpleThing);
        setCallThingType(THING_TYPE_FUNC, errorCall);
        setDispatchThingType(THING_TYPE_FUNC, errorCall);

        THING_TYPE_NATIVE_FUNC = createThingType();
        setDestroyThingType(THING_TYPE_NATIVE_FUNC, destroySimpleThing);
        setCallThingType(THING_TYPE_NATIVE_FUNC, nativeFuncCall);
        setDispatchThingType(THING_TYPE_NATIVE_FUNC, errorCall);

        THING_TYPE_ERROR = createThingType();
        setDestroyThingType(THING_TYPE_ERROR, destroyErrorThing);
        setCallThingType(THING_TYPE_ERROR, errorCall);
        setDispatchThingType(THING_TYPE_ERROR, errorCall);

        SYM_ADD = newSymbolId();
        SYM_SUB = newSymbolId();
        SYM_MUL = newSymbolId();
        SYM_DIV = newSymbolId();
        SYM_EQ = newSymbolId();
        SYM_NOT_EQ = newSymbolId();
        SYM_LESS_THAN = newSymbolId();
        SYM_LESS_THAN_EQ = newSymbolId();
        SYM_GREATER_THAN = newSymbolId();
        SYM_GREATER_THAN_EQ = newSymbolId();
        SYM_ADD = newSymbolId();
        SYM_OR = newSymbolId();
        SYM_NOT = newSymbolId();

        initialized = 1;
    }
}

void deinitThing() {
    if(initialized) {
        free(THING_TYPE_NONE);
        THING_TYPE_NONE = NULL;

        free(THING_TYPE_INT);
        THING_TYPE_INT = NULL;

        free(THING_TYPE_STR);
        THING_TYPE_STR = NULL;

        free(THING_TYPE_BOOL);
        THING_TYPE_BOOL = NULL;

        free(THING_TYPE_SYMBOL);
        THING_TYPE_SYMBOL = NULL;

        free(THING_TYPE_OBJ);
        THING_TYPE_OBJ = NULL;

        free(THING_TYPE_FUNC);
        THING_TYPE_FUNC = NULL;

        free(THING_TYPE_NATIVE_FUNC);
        THING_TYPE_NATIVE_FUNC = NULL;

        free(THING_TYPE_ERROR);
        THING_TYPE_ERROR = NULL;

        SYM_ADD = 0;
        SYM_SUB = 0;
        SYM_MUL = 0;
        SYM_DIV = 0;
        SYM_EQ = 0;
        SYM_LESS_THAN = 0;
        SYM_LESS_THAN_EQ = 0;
        SYM_GREATER_THAN = 0;
        SYM_GREATER_THAN_EQ = 0;
        SYM_ADD = 0;
        SYM_OR = 0;
        SYM_NOT = 0;

        initialized = 0;
    }
}

Thing* thingHeaderToCustomData(ThingHeader* header) {
    return ((char*) header) + sizeof(ThingHeader);
}

ThingHeader* customDataToThingHeader(Thing* thing) {
    return (ThingHeader*) (((char*) thing) - sizeof(ThingHeader));
}

/**
 * Creates a thing. If the runtime is destroyed, the object will also be
 * destroyed.
 *
 * @param runtime the runtime object
 * @param type the native type of the object
 * @param size the size of the object's custom data. The custom data is found
 *      after the Thing struct.
 */
Thing* createThing(Runtime* runtime, ThingType* type, size_t size) {
    ThingHeader* header = malloc(sizeof(ThingHeader) + size);
    header->type = type;
    Thing* thing = thingHeaderToCustomData(header);
    runtime->allocatedThings = consList(thing, runtime->allocatedThings);
    return thing;
}

void destroyThing(Thing* thing) {
    ThingHeader* header = customDataToThingHeader(thing);
    header->type->destroy((Thing*) thing);
    free(header);
}

ThingType* typeOfThing(Thing* thing) {
    return customDataToThingHeader(thing)->type;
}

/**
 * Singleton none object.
 */
typedef struct {
    //structs must have nonzero length.
    char dummy;
} NoneThing;

Thing* createNoneThing(Runtime* runtime) {
    NoneThing* thing = createThing(runtime, THING_TYPE_NONE, sizeof(NoneThing));
    thing->dummy = 0;
    return thing;
}

/**
 * Represents integers found in the source code.
 */
typedef struct {
    int32_t value;
} IntThing;

Thing* createIntThing(Runtime* runtime, int32_t value) {
    IntThing* thing = createThing(runtime, THING_TYPE_INT, sizeof(IntThing));
    thing->value = value;
    return thing;
}

RetVal intDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 2, THING_TYPE_INT, THING_TYPE_INT);
    if(isRetValError(retVal)) {
        return retVal;
    }

    int32_t valueA = thingAsInt(args[0]);
    int32_t valueB = thingAsInt(args[1]);
    uint32_t id = getSymbolId(self);

    Thing* thing = NULL;
    const char* error = NULL;

    if(id == SYM_ADD) {
        thing = createIntThing(runtime, valueA + valueB);
    } else if(id == SYM_SUB) {
        thing = createIntThing(runtime, valueA - valueB);
    } else if(id == SYM_MUL) {
        thing = createIntThing(runtime, valueA * valueB);
    } else if(id == SYM_DIV) {
        thing = createIntThing(runtime, valueA / valueB);
    } else if(id == SYM_EQ) {
        thing = createBoolThing(runtime, (uint8_t) (valueA == valueB));
    } else if(id == SYM_NOT_EQ) {
        thing = createBoolThing(runtime, (uint8_t) (valueA != valueB));
    } else if(id == SYM_LESS_THAN) {
        thing = createBoolThing(runtime, (uint8_t) (valueA < valueB));
    } else if(id == SYM_LESS_THAN_EQ) {
        thing = createBoolThing(runtime, (uint8_t) (valueA <= valueB));
    } else if(id == SYM_GREATER_THAN) {
        thing = createBoolThing(runtime, (uint8_t) (valueA > valueB));
    } else if(id == SYM_GREATER_THAN_EQ) {
        thing = createBoolThing(runtime, (uint8_t) (valueA >= valueB));
    } else {
        //TODO report the symbol
        error = "bools do not respond to that symbol";
    }

    if(error != NULL) {
        return throwMsg(runtime, error);
    } else {
        return createRetVal(thing, 0);
    }
}

int32_t thingAsInt(Thing* thing) {
    return ((IntThing*) thing)->value;
}

typedef struct {
    const char* value;
    uint8_t literal;
} StrThing;

Thing* createStrThing(Runtime* runtime, const char* value, uint8_t literal) {
    StrThing* thing = createThing(runtime, THING_TYPE_STR, sizeof(StrThing));
    thing->value = value;
    thing->literal = literal != 0; // !=0 makes the value a one or a zero
    return thing;
}

void destroyStrThing(Thing* self) {
    StrThing* str = (StrThing*) self;
    if(!str->literal) {
        free((char*) str->value);
    }
}

RetVal strDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 2, THING_TYPE_STR, THING_TYPE_STR);
    if(isRetValError(retVal)) {
        return retVal;
    }

    const char* valueA = thingAsStr(args[0]);
    const char* valueB = thingAsStr(args[1]);
    uint32_t id = getSymbolId(self);

    Thing* thing;
    const char* error = 0;

    if(id == SYM_ADD) {
        uint32_t len = strlen(valueA) + strlen(valueB);
        char* out = malloc(sizeof(char) * (len + 1));
        out[0] = 0;
        strcat(out, valueA);
        strcat(out, valueB);
        thing = createStrThing(runtime, out, 0);
    } else if(id == SYM_EQ) {
        thing = createBoolThing(runtime, strcmp(valueA, valueB) == 0);
    } else if(id == SYM_NOT_EQ) {
        thing = createBoolThing(runtime, strcmp(valueA, valueB) != 0);
    } else {
        //TODO report the symbol
        error = "strs do not respond to that symbol";
    }

    if(error != NULL) {
        return throwMsg(runtime, error);
    } else {
        return createRetVal(thing, 0);
    }
}

const char* thingAsStr(Thing* self) {
    return ((StrThing*) self)->value;
}

typedef struct {
    uint8_t value;
} BoolThing;

Thing* createBoolThing(Runtime* runtime, uint8_t value) {
    BoolThing* thing = createThing(runtime, THING_TYPE_BOOL, sizeof(BoolThing));
    thing->value = value != 0; //ensure the value is 0 or 1
    return thing;
}

RetVal boolDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(typeOfThing(self) != THING_TYPE_SYMBOL) {
        return throwMsg(runtime, "internal error: self should be a symbol");
    }

    uint32_t id = getSymbolId(self);

    Thing* thing;
    const char* error = NULL;

    if(id == SYM_NOT) {
        RetVal retVal = typeCheck(runtime, self, args, arity, 1, THING_TYPE_BOOL);
        if(isRetValError(retVal)) {
            return retVal;
        }

        thing = createBoolThing(runtime, !thingAsBool(args[0]));
    } else {
        RetVal retVal = typeCheck(runtime, self, args, arity, 2, THING_TYPE_BOOL, THING_TYPE_BOOL);
        if(isRetValError(retVal)) {
            return retVal;
        }

        uint8_t valueA = thingAsBool(args[0]);
        uint8_t valueB = thingAsBool(args[1]);

        if(id == SYM_ADD) {
            thing = createBoolThing(runtime, valueA && valueB);
        } else if(id == SYM_OR) {
            thing = createBoolThing(runtime, valueA || valueB);
        } else {
            //TODO report the symbol
            error = newStr("bools do not respond to that symbol");
        }
    }

    if(error != NULL) {
        return throwMsg(runtime, error);
    } else {
        return createRetVal(thing, 0);
    }
}

uint8_t thingAsBool(Thing* thing) {
    return ((BoolThing*) thing)->value;
}

uint32_t symbolId = 0;

uint32_t newSymbolId() {
    return symbolId++;
}

typedef struct {
    uint32_t id;
    uint8_t arity;
} SymbolThing;

Thing* createSymbolThing(Runtime* runtime, uint32_t id, uint8_t arity) {
    SymbolThing* thing = createThing(runtime, THING_TYPE_SYMBOL,
            sizeof(SymbolThing));
    thing->id = id;
    thing->arity = arity;
    return thing;
}

uint32_t getSymbolId(Thing* self) {
    return ((SymbolThing*) self)->id;
}

//not properly supported yet
RetVal symbolCall(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    uint8_t expected = ((SymbolThing*) self)->arity;
    if(arity != expected) {
        const char* format = "expected %i arguments, but got %i";
        return throwMsg(runtime, formatStr(format, expected, arity));
    }
    return typeOfThing(args[0])->dispatch(runtime, self, args, arity);
}

/**
 * Used for module objects and object literals. Associates strings with things.
 */
typedef struct {
    Map* properties;
} ObjectThing;

Thing* createObjectThingFromMap(Runtime* runtime, Map* map) {
    ObjectThing* thing = createThing(runtime, THING_TYPE_OBJ,
            sizeof(ObjectThing));
    thing->properties = copyMap(map);
    return thing;
}

void destroyObjectThing(Thing* thing) {
    destroyMap(((ObjectThing*) thing)->properties, nothing, nothing);
}

Thing* getObjectProperty(Thing* thing, const char* name) {
    return getMapStr(((ObjectThing*) thing)->properties, name);
}

Thing* createFuncThing(Runtime* runtime, uint32_t entry,
        Module* module, Scope* parentScope) {
    FuncThing* thing = createThing(runtime, THING_TYPE_FUNC, sizeof(FuncThing));
    thing->entry = entry;
    thing->module = module;
    thing->parentScope = parentScope;
    return thing;
}

typedef struct {
    ExecFunc func;
} NativeFuncThing;

Thing* createNativeFuncThing(Runtime* runtime, ExecFunc func) {
    NativeFuncThing* thing = createThing(runtime, THING_TYPE_NATIVE_FUNC,
            sizeof(NativeFuncThing));
    thing->func = func;
    return thing;
}

RetVal nativeFuncCall(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return ((NativeFuncThing*) self)->func(runtime, self, args, arity);
}

typedef struct {
    //TODO add file path
    uint8_t native;
    SrcLoc location;
} ErrorFrame;

typedef struct {
    const char* msg;
    List* stackFrame;
} ErrorThing;

Thing* createErrorThing(Runtime* runtime, const char* msg) {
    ErrorThing* self = createThing(runtime, THING_TYPE_ERROR, sizeof(ErrorThing));
    self->msg = msg;
    self->stackFrame = NULL;

    List* list = reverseList(runtime->stackFrame);
    List* listOriginal = list;

    while(list != NULL) {
        StackFrame* stackFrame = list->head;
        ErrorFrame* errorFrame = malloc(sizeof(ErrorFrame));

        errorFrame->location.line = 0;
        errorFrame->location.column = 0;

        if(stackFrame->type == STACK_FRAME_DEF) {
            errorFrame->native = 0;

            StackFrameDef* frameDef = &stackFrame->def;

            for(uint32_t i = 0; i < frameDef->module->srcLocLength; i++) {
                if(frameDef->index == frameDef->module->srcLoc[i].index) {
                    errorFrame->location = frameDef->module->srcLoc[i].location;
                    break;
                }
            }
        } else {
            errorFrame->native = 1;
        }

        self->stackFrame = consList(errorFrame, self->stackFrame);

        list = list->tail;
    }

    destroyList(listOriginal, nothing);

    return self;
}

void destroyErrorThing(Thing* thing) {
    ErrorThing* self = (ErrorThing*) thing;
    free((char*) self->msg);
    destroyList(self->stackFrame, free);
}

const char* errorStackTrace(Runtime* runtime, Thing* self) {
    UNUSED(runtime);

    ErrorThing* error = (ErrorThing*) self;
    const char* footer = formatStr("\terror: %s", error->msg);
    List* parts = consList((char*) footer, NULL);
    size_t length = strlen(footer) + 2;
    List* list = error->stackFrame;

    while(list != NULL) {
        ErrorFrame* frame = list->head;

        char* line;
        if(frame->native) {
            line = newStr("[native code]");
        } else {
            line = (char*) formatStr("at %i, %i",
                frame->location.line, frame->location.column);
        }
        length += strlen(line) + 2;
        parts = consList(line, parts);
        list = list->tail;
    }

    const char* header = "Traceback:";
    parts  = consList(newStr(header), parts);
    length += strlen(header) + 2;

    char* joined = malloc(length + 1);
    joined[0] = 0;
    List* partsOriginal = parts;

    while(parts != NULL) {
        strcat(joined, "\t");
        strcat(joined, parts->head);
        strcat(joined, "\n");
        parts = parts->tail;
    }
    joined[length] = 0;

    destroyList(partsOriginal, free);
    return joined;
}

RetVal typeCheck(Runtime* runtime, Thing* self, Thing** args, uint8_t arity, uint8_t expectedArity, ...) {
    /*if(typeOfThing(self) != THING_TYPE_SYMBOL) {
        return throwMsg(runtime, "internal error: self should be a symbol");
    }*/

    if(arity != expectedArity) {
        const char* msg = formatStr("expected %i arguments but got %i", expectedArity, arity);
        return throwMsg(runtime, msg);
    }

    va_list types;
    va_start(types, expectedArity);


    for(uint8_t i = 0; i < arity; i++) {
        if(typeOfThing(args[i]) != va_arg(types, ThingType*)) {
            va_end(types);
            return throwMsg(runtime, formatStr("wrong type for argument %i", i + 1));
        }
    }

    va_end(types);

    return createRetVal(NULL, 0);
}

