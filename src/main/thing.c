#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "main/execute.h"
#include "main/thing.h"

#define UNUSED(x) (void)(x)

//TODO check if symbol is supported for a dispatch before typechecking
//for better error messages

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
RetVal floatDispatch(Runtime*, Thing*, Thing**, uint8_t);
RetVal strDispatch(Runtime*, Thing*, Thing**, uint8_t);
void destroyStrThing(Thing*);
RetVal boolDispatch(Runtime*, Thing*, Thing**, uint8_t);
uint32_t getSymbolId(Thing* self);
RetVal nativeFuncCall(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
RetVal tupleDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
//RetVal listDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
RetVal objectDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
RetVal consCall(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
RetVal consDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
RetVal moduleDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

void destroyModuleThing(Thing* thing);
void destroyErrorThing(Thing* thing);
void destroyTupleThing(Thing* thing);
void destroyObjectThing(Thing* thing);

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

        THING_TYPE_FLOAT = createThingType();
        setDestroyThingType(THING_TYPE_FLOAT, destroySimpleThing);
        setCallThingType(THING_TYPE_FLOAT, errorCall);
        setDispatchThingType(THING_TYPE_FLOAT, floatDispatch);

        THING_TYPE_STR = createThingType();
        setDestroyThingType(THING_TYPE_STR, destroyStrThing);
        setCallThingType(THING_TYPE_STR, errorCall);
        setDispatchThingType(THING_TYPE_STR, strDispatch);

        THING_TYPE_BOOL = createThingType();
        setDestroyThingType(THING_TYPE_BOOL, destroySimpleThing);
        setCallThingType(THING_TYPE_BOOL, errorCall);
        setDispatchThingType(THING_TYPE_BOOL, boolDispatch);

        THING_TYPE_MODULE = createThingType();
        setDestroyThingType(THING_TYPE_MODULE, destroyModuleThing);
        setCallThingType(THING_TYPE_MODULE, errorCall);
        setDispatchThingType(THING_TYPE_MODULE, moduleDispatch);

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

        THING_TYPE_TUPLE = createThingType();
        setDestroyThingType(THING_TYPE_TUPLE, destroyTupleThing);
        setCallThingType(THING_TYPE_TUPLE, errorCall);
        setDispatchThingType(THING_TYPE_TUPLE, tupleDispatch);

        THING_TYPE_LIST = createThingType();
        setDestroyThingType(THING_TYPE_LIST, destroySimpleThing);
        setCallThingType(THING_TYPE_LIST, errorCall);
        setDispatchThingType(THING_TYPE_LIST, errorCall);

        THING_TYPE_OBJECT = createThingType();
        setDestroyThingType(THING_TYPE_OBJECT, destroyObjectThing);
        setCallThingType(THING_TYPE_OBJECT, errorCall);
        setDispatchThingType(THING_TYPE_OBJECT, objectDispatch);

        THING_TYPE_CELL = createThingType();
        setDestroyThingType(THING_TYPE_CELL, destroySimpleThing);
        setCallThingType(THING_TYPE_CELL, errorCall);
        setDispatchThingType(THING_TYPE_CELL, errorCall);

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
        SYM_GET = newSymbolId();
        SYM_DOT = newSymbolId();

        initialized = 1;
    }
}

void deinitThing() {
    if(initialized) {
        free(THING_TYPE_NONE);
        THING_TYPE_NONE = NULL;

        free(THING_TYPE_INT);
        THING_TYPE_INT = NULL;

        free(THING_TYPE_FLOAT);
        THING_TYPE_FLOAT = NULL;

        free(THING_TYPE_STR);
        THING_TYPE_STR = NULL;

        free(THING_TYPE_BOOL);
        THING_TYPE_BOOL = NULL;

        free(THING_TYPE_SYMBOL);
        THING_TYPE_SYMBOL = NULL;

        free(THING_TYPE_MODULE);
        THING_TYPE_MODULE = NULL;

        free(THING_TYPE_FUNC);
        THING_TYPE_FUNC = NULL;

        free(THING_TYPE_NATIVE_FUNC);
        THING_TYPE_NATIVE_FUNC = NULL;

        free(THING_TYPE_ERROR);
        THING_TYPE_ERROR = NULL;

        free(THING_TYPE_TUPLE);
        THING_TYPE_TUPLE = NULL;

        free(THING_TYPE_LIST);
        THING_TYPE_LIST = NULL;

        free(THING_TYPE_OBJECT);
        THING_TYPE_OBJECT = NULL;

        free(THING_TYPE_CELL);
        THING_TYPE_CELL = NULL;

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
        SYM_GET = 0;
        SYM_DOT = 0;

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
        error = "ints do not respond to that symbol";
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
    float value;
} FloatThing;

Thing* createFloatThing(Runtime* runtime, float value) {
    FloatThing* thing = createThing(runtime, THING_TYPE_FLOAT, sizeof(FloatThing));
    thing->value = value;
    return thing;
}

RetVal floatDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    UNUSED(self);

    RetVal retVal = typeCheck(runtime, self, args, arity, 2, THING_TYPE_FLOAT, THING_TYPE_FLOAT);
    if(isRetValError(retVal)) {
        return retVal;
    }

    float valueA = thingAsInt(args[0]);
    float valueB = thingAsInt(args[1]);
    uint32_t id = getSymbolId(self);

    Thing* thing = NULL;
    const char* error = NULL;

    if(id == SYM_ADD) {
        thing = createFloatThing(runtime, valueA + valueB);
    } else if(id == SYM_SUB) {
        thing = createFloatThing(runtime, valueA - valueB);
    } else if(id == SYM_MUL) {
        thing = createFloatThing(runtime, valueA * valueB);
    } else if(id == SYM_DIV) {
        thing = createFloatThing(runtime, valueA / valueB);
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
        error = "floats do not respond to that symbol";
    }

    if(error != NULL) {
        return throwMsg(runtime, error);
    } else {
        return createRetVal(thing, 0);
    }
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

//TODO make bools respond to == and !=
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

        //TODO make this SYM_AND
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
} ModuleThing;

Thing* createModuleThing(Runtime* runtime, Map* map) {
    ModuleThing* thing = createThing(runtime, THING_TYPE_MODULE,
            sizeof(ModuleThing));
    thing->properties = copyMap(map);
    return thing;
}

void destroyModuleThing(Thing* thing) {
    destroyMap(((ModuleThing*) thing)->properties, nothing, nothing);
}

Thing* getModuleProperty(Thing* thing, const char* name) {
    return getMapStr(((ModuleThing*) thing)->properties, name);
}

RetVal moduleDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    RetVal ret = typeCheck(runtime, self, args, arity, 2, THING_TYPE_MODULE,
            THING_TYPE_STR);
    if(isRetValError(ret)) {
        return ret;
    }

    if(getSymbolId(self) != SYM_DOT) {
        //TODO report the actual symbol
        const char* msg = newStr("modules do not respond to that symbol");
        return throwMsg(runtime, msg);
    }

    const char* name = thingAsStr(args[1]);
    Thing* property = getModuleProperty(args[0], name);
    if(property == NULL) {
        return throwMsg(runtime, formatStr("export '%s' not found", name));
    }

    return createRetVal(property, 0);
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

RetVal typeCheck(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t expectedArity, ...) {
    UNUSED(self);

    if(arity != expectedArity) {
        const char* msg = formatStr("expected %i arguments but got %i",
                expectedArity, arity);
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

typedef struct {
    uint8_t size;
    Thing** elements;
} TupleThing;

Thing* createTupleThing(Runtime* runtime, uint8_t size, Thing** elements) {
    TupleThing* self = createThing(runtime, THING_TYPE_TUPLE, sizeof(TupleThing));
    self->size = size;
    self->elements = elements;
    return self;
}

uint8_t getTupleSize(Thing* tuple) {
    return ((TupleThing*) tuple)->size;
}

Thing* getTupleElem(Thing* tuple, uint8_t index) {
    return ((TupleThing*) tuple)->elements[index];
}

RetVal tupleDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    //TODO check self is really a symbol first
    uint32_t id = getSymbolId(self);

    if(id == SYM_EQ || id == SYM_NOT_EQ) {
        RetVal ret = typeCheck(runtime, self, args, arity, 2, THING_TYPE_TUPLE,
                THING_TYPE_TUPLE);

        if(isRetValError(ret)) {
            return ret;
        }
        uint8_t all = getSymbolId(self) == SYM_EQ;

        TupleThing* valueA = (TupleThing*) args[0];
        TupleThing* valueB = (TupleThing*) args[1];

        if(valueA->size != valueB->size) {
            return createRetVal(createBoolThing(runtime, 0), 0);
        }

        Thing* eqSym = getMapStr(runtime->operators, "==");

        Thing** elems = malloc(sizeof(Thing*) * 2);

        for(uint8_t i = 0; i < valueA->size; i++) {
            elems[0] = valueA->elements[i];
            elems[1] = valueB->elements[i];

            ret = callFunction(runtime, eqSym, 2, elems);
            if(isRetValError(ret)) {
                free(elems);
                return ret;
            }

            Thing* value = getRetVal(ret);

            if(typeOfThing(value) != THING_TYPE_BOOL) {
                free(elems);
                const char* msg = newStr("internal error: == did not return a bool");
                return throwMsg(runtime, msg);
            }

            if(thingAsBool(value) != all) {
                free(elems);
                return ret;
            }
        }

        free(elems);
        return createRetVal(createBoolThing(runtime, all), 0);
    } else if(id == SYM_GET) {
        RetVal ret = typeCheck(runtime, self, args, arity, 2,
                THING_TYPE_TUPLE, THING_TYPE_INT);

        if(isRetValError(ret)) {
            return ret;
        }

        TupleThing* tuple = (TupleThing*) args[0];
        int32_t index = thingAsInt(args[1]);

        if(index >= tuple->size) {
            const char* format = "tuple access out of bounds: "
                    "accessed at %i but the size is %i";
            return throwMsg(runtime, formatStr(format, index, tuple->size));
        }

        return createRetVal(tuple->elements[index], 0);
    } else {
        return throwMsg(runtime, newStr("tuples do not respond to that symbol"));
    }
}

void destroyTupleThing(Thing* thing) {
    free(((TupleThing*) thing)->elements);
}

Thing* createListThing(Runtime* runtime, Thing* head, Thing* tail) {
    ListThing* list = createThing(runtime, THING_TYPE_LIST, sizeof(ListThing));
    list->head = head;
    list->tail = tail;
    return list;
}

typedef struct {
    Map* map;
} ObjectThing;

Thing* createObjectThing(Runtime* runtime, Map* map) {
    ObjectThing* object = createThing(runtime, THING_TYPE_OBJECT,
            sizeof(ObjectThing));
    object->map = map;
    return object;
}

RetVal objectDispatch(Runtime* runtime, Thing* self, Thing** args,
        uint8_t arity) {
    RetVal ret = typeCheck(runtime, self, args, arity, 1, THING_TYPE_OBJECT);
    if(isRetValError(ret)) {
        return ret;
    }
    ObjectThing* object = args[0];
    Thing* value = getMapUint32(object->map, getSymbolId(self));
    if(value == NULL) {
        return throwMsg(runtime, newStr("object does not respond to that symbol"));
    } else {
        return createRetVal(value, 0);
    }
}

void destroyObjectThing(Thing* self) {
    ObjectThing* object = (ObjectThing*) self;
    destroyMap(object->map, free, nothing);
}

typedef struct {
    Thing* value;
} CellThing;

Thing* createCellThing(Runtime* runtime, Thing* value) {
    CellThing* cell = createThing(runtime, THING_TYPE_CELL, sizeof(CellThing));
    cell->value = value;
    return cell;
}

Thing* getCellValue(Thing* cell) {
    return ((CellThing*) cell)->value;
}

void setCellValue(Thing* cell, Thing* value) {
    ((CellThing*) cell)->value = value;
}

/*RetVal listDispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    if(typeOfThing(self) != THING_TYPE_SYMBOL) {
        throwMsg(runtime, newStr("internal error: self is not a symbol"));
    }

    uint8_t id = getSymbolId(self);

    if(id != SYM_EQ && id != SYM_NOT_EQ) {
        throwMsg(runtime, newStr("lists only respond to == and !="));
    }

    if(arity != 2) {
        throwMsg(runtime, formatStr("expected 2 arguments but got %i", arity));
    }

    ThingType* type1 = typeOfThing(args[0]);
    if(type1 != THING_TYPE_LIST && type1 != THING_TYPE_NONE) {
        //TODO report the actual type
        throwMsg(runtime, newStr("expected argument 1 to be none or a list"));
    }

    ThingType* type2 = typeOfThing(args[1]);
    if(type2 != THING_TYPE_LIST && type2 != THING_TYPE_NONE) {
        //TODO report the actual type
        throwMsg(runtime, newStr("expected arguments 1 to be none or a list"));
    }

    if(type1 != type2) {
        return createRetVal(createBoolThing(runtime, 0), 0);
    } else if(type1 == THING_TYPE_NONE) {
        return createRetVal(createBoolThing(runtime, 1), 0);
    } else {
        const char* opName;
        uint8_t all;
        if(id == SYM_EQ) {
            opName = "==";
            all = 1;
        } else {
            opName = "!=";
            all = 0;
        }

        Thing* operator = getMapStr(runtime->operators, opName);

        ListThing* list1 = (ListThing*) args[0];
        ListThing* list2 = (ListThing*) args[1];

        Thing** passed = malloc(sizeof(Thing*) * 2);
        passed[0] = list1->head;
        passed[1] = list2->head;

        RetVal ret = callFunction(runtime, operator, 2, passed);
        if(isRetValError(ret)) {
            free(passed);
            return ret;
        }

        Thing* value = getRetVal(ret);

        if(typeOfThing(value) != THING_TYPE_BOOL) {
            free(passed);
            //TODO report which symbol
            throwMsg(runtime, newStr("symbol did not return a bool"));
        }

        if(thingAsBool(value) != all) {
            free(passed);
            return createRetVal(createBoolThing(runtime, 0), 0);
        }

        passed[0] = list1->tail;
        passed[1] = list2->tail;

        ret = callFunction(runtime, operator, 2, passed);
        if(isRetValError(ret)) {
            free(passed);
            return ret;
        }

        value = getRetVal(ret);
        if(typeOfThing(value) != THING_TYPE_BOOL) {
            free(passed);
            //TODO report which symbol
            throwMsg(runtime, newStr("symbol did not return a bool"));
        }

        free(passed);
        return createRetVal(createBoolThing(runtime, thingAsBool(value) == all), 0);
    }
}*/
