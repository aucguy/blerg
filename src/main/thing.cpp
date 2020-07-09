#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "main/execute.h"
#include "main/runtime.h"
#include "main/thing.h"

float thingAsFloat(Thing* thing);

RetVal callFail(Runtime* runtime) {
    return throwMsg(runtime, "cannot call this type");
}

class NoneThing : public Thing {
public:
    NoneThing() {}
    ~NoneThing() {}

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    ThingTypes type() {
        return TYPE_NONE;
    }
};

//TODO rename function
RetVal symbolDispatch(Runtime* runtime, Thing* self, Thing** args,
        uint8_t arity) {
    if(getSymbolId(self) == SYM_RESPONDS_TO) {
        if(arity != 2) {
            const char* fmt = "expected 2 args but got %i";
            return throwMsg(runtime, formatStr(fmt, arity));
        }

        if(typeOfThing(args[1]) != TYPE_SYMBOL) {
            //TODO report the actual type
            const char* msg = "expected argument 2 to be a symbol";
            return throwMsg(runtime, newStr(msg));
        }

        uint8_t respondsTo = getSymbolId(args[1]) == SYM_RESPONDS_TO;

        return createRetVal(createBoolThing(runtime, respondsTo), 0);
    } else {
        //TODO report the actual symbol
        return throwMsg(runtime, newStr("symbols to not respond to that symbol"));
    }
}

class SymbolThing : public Thing {
public:
    uint32_t id;
    uint8_t arity;

    SymbolThing(uint32_t id, uint8_t arity) :
        id(id), arity(arity) {}
    ~SymbolThing() {}

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        //TODO remove this check
        if(arity != this->arity && arity != 0) {
            const char* format = "expected %i arguments, but got %i";
            return throwMsg(runtime, formatStr(format, this->arity, arity));
        }
        return args[0]->dispatch(runtime, self, args, arity);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return symbolDispatch(runtime, self, args, arity);
    }

    ThingTypes type() {
        return TYPE_SYMBOL;
    }
};

class IntThing : public Thing {
public:
    int32_t value;

    IntThing(int32_t value) :
        value(value) {}
    ~IntThing() {}

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        uint32_t id = getSymbolId(self);

        if(id == SYM_RESPONDS_TO) {
            if(arity != 2) {
                throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
            }

            if(typeOfThing(args[1]) != TYPE_SYMBOL) {
                //TODO report the type
                throwMsg(runtime, formatStr("expected argument 2 to be a symbol"));
            }

            uint32_t checking = getSymbolId(args[1]);

            uint32_t respondsTo =
                    checking == SYM_RESPONDS_TO ||
                    checking == SYM_ADD ||
                    checking == SYM_SUB ||
                    checking == SYM_MUL ||
                    checking == SYM_DIV ||
                    checking == SYM_EQ ||
                    checking == SYM_NOT_EQ ||
                    checking == SYM_LESS_THAN ||
                    checking == SYM_LESS_THAN_EQ ||
                    checking == SYM_GREATER_THAN ||
                    checking == SYM_GREATER_THAN_EQ;

            return createRetVal(createBoolThing(runtime, respondsTo), 0);
        } else {
            RetVal retVal = typeCheck(runtime, self, args, arity, 2, TYPE_INT,
                    TYPE_INT);
            if(isRetValError(retVal)) {
                return retVal;
            }

            int32_t valueA = thingAsInt(args[0]);
            int32_t valueB = thingAsInt(args[1]);

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
                //this case shouldn't happen
                error = "ints do not respond to that symbol";
            }

            if(error != NULL) {
                return throwMsg(runtime, error);
            } else {
                return createRetVal(thing, 0);
            }
        }
    }

    ThingTypes type() {
        return TYPE_INT;
    }
};

class FloatThing : public Thing {
public:
    float value;

    FloatThing(float value) :
        value(value) {}

    ~FloatThing() {}

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        uint32_t id = getSymbolId(self);
        if(id == SYM_RESPONDS_TO) {
            if(arity != 2) {
                const char* fmt = "expected 2 args but got %i";
                return throwMsg(runtime, formatStr(fmt, arity));
            }

            if(typeOfThing(args[1]) != TYPE_SYMBOL) {
                //TODO report the actual type
                const char* msg = "expected argument 2 to be a symbol";
                return throwMsg(runtime, newStr(msg));
            }

            uint32_t checking = getSymbolId(args[1]);
            uint8_t respondsTo =
                    checking == SYM_ADD ||
                    checking == SYM_SUB ||
                    checking == SYM_MUL ||
                    checking == SYM_DIV ||
                    checking == SYM_EQ ||
                    checking == SYM_NOT_EQ ||
                    checking == SYM_LESS_THAN ||
                    checking == SYM_LESS_THAN_EQ ||
                    checking == SYM_GREATER_THAN ||
                    checking == SYM_GREATER_THAN_EQ;

            return createRetVal(createBoolThing(runtime, respondsTo), 0);
        } else {
            RetVal retVal = typeCheck(runtime, self, args, arity, 2,
                    TYPE_FLOAT, TYPE_FLOAT);
            if(isRetValError(retVal)) {
                return retVal;
            }

            float valueA = thingAsFloat(args[0]);
            float valueB = thingAsFloat(args[1]);

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
    }

    ThingTypes type() {
        return TYPE_FLOAT;
    }
};

class StrThing : public Thing {
public:
    const char* value;
    uint8_t literal;

    StrThing(const char* value, uint8_t literal) :
        value(value), literal(literal) {}

    ~StrThing() {
        if(!this->literal) {
            free((char*) this->value);
        }
    }

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        uint32_t id = getSymbolId(self);

        if(id == SYM_RESPONDS_TO) {
            if(arity != 2) {
                const char* fmt = "expected 2 args but got %i";
                return throwMsg(runtime, formatStr(fmt, arity));
            }

            if(typeOfThing(args[1]) != TYPE_SYMBOL) {
                //TODO report the actual type
                const char* msg = "expected argument 2 to be a symbol";
                return throwMsg(runtime, newStr(msg));
            }

            uint32_t checking = getSymbolId(args[1]);
            uint8_t respondsTo =
                    checking == SYM_ADD ||
                    checking == SYM_EQ ||
                    checking == SYM_NOT_EQ;

            return createRetVal(createBoolThing(runtime, respondsTo), 0);
        } else {
            RetVal retVal = typeCheck(runtime, self, args, arity, 2, TYPE_STR,
                    TYPE_STR);
            if(isRetValError(retVal)) {
                return retVal;
            }

            const char* valueA = thingAsStr(args[0]);
            const char* valueB = thingAsStr(args[1]);

            Thing* thing;
            const char* error = 0;

            if(id == SYM_ADD) {
                uint32_t len = strlen(valueA) + strlen(valueB);
                char* out = (char*) malloc(sizeof(char) * (len + 1));
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
    }

    ThingTypes type() {
        return TYPE_STR;
    }
};

class BoolThing : public Thing {
public:
    uint8_t value;

    BoolThing(uint8_t value) :
        value(value) {}

    ~BoolThing() {}

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        if(typeOfThing(self) != TYPE_SYMBOL) {\
            const char* msg = "internal error: self should be a symbol";
            return throwMsg(runtime, newStr(msg));
        }

        uint32_t id = getSymbolId(self);

        if(id == SYM_RESPONDS_TO) {
            if(arity != 2) {
                const char* fmt = "expected 2 args but got %i";
                return throwMsg(runtime, formatStr(fmt, arity));
            }

            if(typeOfThing(args[1]) != TYPE_SYMBOL) {
                //TODO report the actual type
                const char* msg = "expected argument 2 to be a symbol";
                return throwMsg(runtime, newStr(msg));
            }

            uint32_t checking = getSymbolId(args[1]);
            uint8_t respondsTo =
                    checking == SYM_NOT ||
                    checking == SYM_ADD ||
                    checking == SYM_NOT;

            return createRetVal(createBoolThing(runtime, respondsTo), 0);
        } else {
            Thing* thing;
            const char* error = NULL;

            if(id == SYM_NOT) {
                RetVal retVal = typeCheck(runtime, self, args, arity, 1,
                        TYPE_BOOL);
                if(isRetValError(retVal)) {
                    return retVal;
                }

                thing = createBoolThing(runtime, !thingAsBool(args[0]));
            } else {
                RetVal retVal = typeCheck(runtime, self, args, arity, 2,
                        TYPE_BOOL, TYPE_BOOL);
                if(isRetValError(retVal)) {
                    return retVal;
                }

                uint8_t valueA = thingAsBool(args[0]);
                uint8_t valueB = thingAsBool(args[1]);

                if(id == SYM_AND) {
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
    }

    ThingTypes type() {
        return TYPE_BOOL;
    }
};

class ModuleThing : public Thing {
public:
    Map* properties;

    ModuleThing(Map* properties) :
        properties(properties) {}

    ~ModuleThing() {
        destroyMap(this->properties, nothing, nothing);
    }

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        if(getSymbolId(self) == SYM_RESPONDS_TO) {
            if(arity != 2) {
                return throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
            }

            if(typeOfThing(args[1]) != TYPE_SYMBOL) {
                //TODO report the actual type
                const char* msg = "expected argument 2 to be a symbol";
                return throwMsg(runtime, newStr(msg));
            }

            uint8_t respondsTo = getSymbolId(args[1]) == SYM_DOT;

            return createRetVal(createBoolThing(runtime, respondsTo), 0);
        } else {
            RetVal ret = typeCheck(runtime, self, args, arity, 2, TYPE_MODULE,
                    TYPE_STR);
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
    }

    ThingTypes type() {
        return TYPE_MODULE;
    }
};

class FuncThing : public Thing {
public:
    //location of first bytecode of the function
    unsigned int entry;
    //module the function was declared in
    Module* module;
    //the scope the function was declared in
    Scope* parentScope;

    FuncThing(unsigned int entry, Module* module, Scope* parentScope) :
        entry(entry), module(module), parentScope(parentScope) {}

    ~FuncThing() {}

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return symbolDispatch(runtime, self, args, arity);
    }

    ThingTypes type() {
        return TYPE_FUNC;
    }
};

unsigned int getFuncEntry(Thing* thing) {
    return ((FuncThing*) thing)->entry;
}

Module* getFuncModule(Thing* thing) {
    return ((FuncThing*) thing)->module;
}

Scope* getFuncParentScope(Thing* thing) {
    return ((FuncThing*) thing)->parentScope;
}

class NativeFuncThing : public Thing {
public:
    ExecFunc func;

    NativeFuncThing(ExecFunc func)
        : func(func) {}
    ~NativeFuncThing() {}

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return this->func(runtime, self, args, arity);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return symbolDispatch(runtime, self, args, arity);
    }

    ThingTypes type() {
        return TYPE_NATIVE_FUNC;
    }
};

typedef struct {
    uint8_t native;
    SrcLoc location;
    const char* filename;
} ErrorFrame;

class ErrorThing : public Thing {
public:
    const char* msg;
    List* stackFrame;

    ErrorThing(const char* msg, List* stackFrame) :
        msg(msg), stackFrame(stackFrame) {}

    ~ErrorThing() {
        free((char*) this->msg);
        destroyList(this->stackFrame, free);
    }

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return symbolDispatch(runtime, self, args, arity);
    }

    ThingTypes type() {
        return TYPE_ERROR;
    }
};

class TupleThing : public Thing {
public:
    uint8_t size;
    Thing** elements;

    TupleThing(uint8_t size, Thing** elements) :
        size(size), elements(elements) {}

    ~TupleThing() {
        free(this->elements);
    }

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        //TODO check self is really a symbol first
        uint32_t id = getSymbolId(self);

        if(id == SYM_RESPONDS_TO) {
            if(arity != 2) {
                return throwMsg(runtime, formatStr("expected 2 args but got %i", arity));
            }

            if(typeOfThing(args[1]) != TYPE_SYMBOL) {
                //TODO report the actual type
                const char* msg = "expected argument 2 to be a symbol";
                return throwMsg(runtime, newStr(msg));
            }

            uint32_t checking = getSymbolId(args[1]);
            uint8_t respondsTo =
                    checking == SYM_RESPONDS_TO ||
                    checking == SYM_EQ ||
                    checking == SYM_NOT_EQ ||
                    checking == SYM_GET;

            return createRetVal(createBoolThing(runtime, respondsTo), 0);
        } else if(id == SYM_EQ || id == SYM_NOT_EQ) {
            RetVal ret = typeCheck(runtime, self, args, arity, 2, TYPE_TUPLE,
                    TYPE_TUPLE);

            if(isRetValError(ret)) {
                return ret;
            }
            uint8_t all = getSymbolId(self) == SYM_EQ;

            TupleThing* valueA = (TupleThing*) args[0];
            TupleThing* valueB = (TupleThing*) args[1];

            if(valueA->size != valueB->size) {
                return createRetVal(createBoolThing(runtime, 0), 0);
            }

            Thing* eqSym = (Thing*) getMapStr(runtime->operators, "==");

            Thing** elems = (Thing**) malloc(sizeof(Thing*) * 2);

            for(uint8_t i = 0; i < valueA->size; i++) {
                elems[0] = valueA->elements[i];
                elems[1] = valueB->elements[i];

                ret = callFunction(runtime, eqSym, 2, elems);
                if(isRetValError(ret)) {
                    free(elems);
                    return ret;
                }

                Thing* value = getRetVal(ret);

                if(typeOfThing(value) != TYPE_BOOL) {
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
                    TYPE_TUPLE, TYPE_INT);

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

    ThingTypes type() {
        return TYPE_TUPLE;
    }
};

class ListThing : public Thing {
public:
    Thing* head;
    Thing* tail;

    ListThing(Thing* head, Thing* tail) :
        head(head), tail(tail) {}
    ~ListThing() {}

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    ThingTypes type() {
        return TYPE_LIST;
    }
};

Thing* getListHead(Thing* thing) {
    return ((ListThing*) thing)->head;
}

Thing* getListTail(Thing* thing) {
    return ((ListThing*) thing)->tail;
}

class ObjectThing : public Thing {
public:
    Map* map;

    ObjectThing(Map* map) :
        map(map) {}

    ~ObjectThing() {
        destroyMap(this->map, free, nothing);
    }

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        //these typechecks should pass, but they are here just in case
        if(arity == 0) {
            const char* fmt = "expected at least 1 args but got %i";
            return throwMsg(runtime, formatStr(fmt, arity));
        }

        if(typeOfThing(self) != TYPE_OBJECT) {
            //TODO report the actual type
            return throwMsg(runtime, "expected self to be an object");
        }

        Thing* value = (Thing*) getMapUint32(this->map, SYM_CALL);
        if(value == NULL) {
            const char* msg = "object is not callable (does not have call property)";
            return throwMsg(runtime, newStr(msg));
        } else {
            return callFunction(runtime, value, arity, args);
        }
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        //these typechecks should pass, but they for here just in case
        if(arity == 0) {
            const char* fmt = "expected at least 1 args but got %i";
            return throwMsg(runtime, formatStr(fmt, arity));
        }

        if(typeOfThing(args[0]) != TYPE_OBJECT) {
            //TODO report the actual type
            return throwMsg(runtime, "expected argument 1 to be an object");
        }

        Thing* value = (Thing*) getMapUint32(this->map, getSymbolId(self));
        if(value == NULL) {
            if(getSymbolId(self) == SYM_RESPONDS_TO) {
                if(arity != 2) {
                    const char* fmt = "expected 2 arg but got %i";
                    return throwMsg(runtime, formatStr(fmt, arity));
                }

                if(typeOfThing(args[1]) != TYPE_SYMBOL) {
                    //TODO report the actual type
                    const char* msg = "expected argument 2 to be a symbol";
                    return throwMsg(runtime, newStr(msg));
                }

                uint32_t checking = getSymbolId(args[1]);
                uint32_t responds = getMapUint32(this->map, checking) != NULL;
                return createRetVal(createBoolThing(runtime, responds), 0);
            } else {
                const char* msg = "object does not respond to that symbol";
                return throwMsg(runtime, newStr(msg));
            }
        } else if(arity == 1) {
            return createRetVal(value, 0);
        } else {
            Thing** passedArgs = (Thing**) malloc(sizeof(Thing*) * (arity - 1));
            for(uint8_t i = 1; i < arity; i++) {
                passedArgs[i - 1] = args[i];
            }

            RetVal ret = callFunction(runtime, value, arity - 1, passedArgs);
            free(passedArgs);
            return ret;
        }
    }

    ThingTypes type() {
        return TYPE_OBJECT;
    }
};

class CellThing : public Thing {
public:
    Thing* value;

    CellThing(Thing* value) :
        value(value) {}
    ~CellThing() {}

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return callFail(runtime);
    }

    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
        return symbolDispatch(runtime, self, args, arity);
    }

    ThingTypes type() {
        return TYPE_CELL;
    }
};

uint32_t SYM_ADD = 0;
uint32_t SYM_SUB = 0;
uint32_t SYM_MUL = 0;
uint32_t SYM_DIV = 0;
uint32_t SYM_EQ = 0;
uint32_t SYM_NOT_EQ = 0;
uint32_t SYM_LESS_THAN = 0;
uint32_t SYM_LESS_THAN_EQ = 0;
uint32_t SYM_GREATER_THAN = 0;
uint32_t SYM_GREATER_THAN_EQ = 0;
uint32_t SYM_AND = 0;
uint32_t SYM_OR = 0;
uint32_t SYM_NOT = 0;
uint32_t SYM_GET = 0;
uint32_t SYM_DOT = 0;
uint32_t SYM_CALL = 0;
uint32_t SYM_RESPONDS_TO = 0;
uint32_t SYM_UNPACK = 0;

#define UNUSED(x) (void)(x)

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

static uint8_t initialized = 0;

void initThing() {
    if(!initialized) {
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
        SYM_AND = newSymbolId();
        SYM_OR = newSymbolId();
        SYM_NOT = newSymbolId();
        SYM_GET = newSymbolId();
        SYM_DOT = newSymbolId();
        SYM_CALL = newSymbolId();
        SYM_RESPONDS_TO = newSymbolId();
        SYM_UNPACK = newSymbolId();

        initialized = 1;
    }
}

void deinitThing() {
    if(initialized) {
        SYM_ADD = 0;
        SYM_SUB = 0;
        SYM_MUL = 0;
        SYM_DIV = 0;
        SYM_EQ = 0;
        SYM_LESS_THAN = 0;
        SYM_LESS_THAN_EQ = 0;
        SYM_GREATER_THAN = 0;
        SYM_GREATER_THAN_EQ = 0;
        SYM_AND = 0;
        SYM_OR = 0;
        SYM_NOT = 0;
        SYM_GET = 0;
        SYM_DOT = 0;
        SYM_CALL = 0;

        initialized = 0;
    }
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
Thing* createThing(Runtime* runtime, Thing* type) {
    runtime->allocatedThings = consList(type, runtime->allocatedThings);
    return type;
}

void destroyThing(Thing* thing) {
    delete thing;
}

ThingTypes typeOfThing(Thing* thing) {
    return thing->type();
}

/**
 * Singleton none object.
 */

Thing* createNoneThing(Runtime* runtime) {
    return createThing(runtime, new NoneThing());
}

/**
 * Represents integers found in the source code.
 */
Thing* createIntThing(Runtime* runtime, int32_t value) {
    return createThing(runtime, new IntThing(value));
}

int32_t thingAsInt(Thing* thing) {
    return ((IntThing*) thing)->value;
}

Thing* createFloatThing(Runtime* runtime, float value) {
    return createThing(runtime, new FloatThing(value));
}

float thingAsFloat(Thing* thing) {
    return ((FloatThing*) thing)->value;
}

Thing* createStrThing(Runtime* runtime, const char* value, uint8_t literal) {
    return createThing(runtime, new StrThing(value, literal));
}

const char* thingAsStr(Thing* self) {
    return ((StrThing*) self)->value;
}

Thing* createBoolThing(Runtime* runtime, uint8_t value) {
    return createThing(runtime, new BoolThing(value));
}

uint8_t thingAsBool(Thing* thing) {
    return ((BoolThing*) thing)->value;
}

uint32_t symbolId = 0;

uint32_t newSymbolId() {
    return symbolId++;
}

Thing* createSymbolThing(Runtime* runtime, uint32_t id, uint8_t arity) {
    return createThing(runtime, new SymbolThing(id, arity));
}

uint32_t getSymbolId(Thing* self) {
    return ((SymbolThing*) self)->id;
}

/**
 * Used for module objects and object literals. Associates strings with things.
 */
Thing* createModuleThing(Runtime* runtime, Map* map) {
    return createThing(runtime, new ModuleThing(copyMap(map)));
}

Thing* getModuleProperty(Thing* thing, const char* name) {
    return (Thing*) getMapStr(((ModuleThing*) thing)->properties, name);
}

Thing* createFuncThing(Runtime* runtime, uint32_t entry,
        Module* module, Scope* parentScope) {
    return createThing(runtime, new FuncThing(entry, module, parentScope));
}

Thing* createNativeFuncThing(Runtime* runtime, ExecFunc func) {
    return createThing(runtime, new NativeFuncThing(func));
}

Thing* createErrorThing(Runtime* runtime, const char* msg) {
    Thing* thing = createThing(runtime, new ErrorThing(msg, NULL));
    ErrorThing* self = (ErrorThing*) thing;

    List* list = reverseList(runtime->stackFrame);
    List* listOriginal = list;

    while(list != NULL) {
        StackFrame* stackFrame = (StackFrame*) list->head;
        ErrorFrame* errorFrame = (ErrorFrame*) malloc(sizeof(ErrorFrame));

        errorFrame->location.line = 0;
        errorFrame->location.column = 0;
        errorFrame->filename = NULL;

        if(stackFrame->type == STACK_FRAME_DEF) {
            errorFrame->native = 0;

            StackFrameDef* frameDef = &stackFrame->def;

            for(uint32_t i = 0; i < frameDef->module->srcLocLength; i++) {
                if(frameDef->index == frameDef->module->srcLoc[i].index) {
                    errorFrame->location = frameDef->module->srcLoc[i].location;
                    break;
                }
            }

            errorFrame->filename = frameDef->module->name;
        } else {
            errorFrame->native = 1;
        }

        self->stackFrame = consList(errorFrame, self->stackFrame);

        list = list->tail;
    }

    destroyList(listOriginal, nothing);

    return thing;
}

const char* errorStackTrace(Runtime* runtime, Thing* self) {
    UNUSED(runtime);

    ErrorThing* error = (ErrorThing*) self;
    const char* footer = formatStr("\terror: %s", error->msg);
    List* parts = consList((char*) footer, NULL);
    size_t length = strlen(footer) + 2;
    List* list = error->stackFrame;

    while(list != NULL) {
        ErrorFrame* frame = (ErrorFrame*) list->head;

        char* line;
        if(frame->native) {
            line = newStr("[native code]");
        } else {
            const char* filename;
            if(frame->filename == NULL) {
                filename = "[native code]";
            } else {
                filename = frame->filename;
            }

            line = (char*) formatStr("%s at %i, %i", filename,
                frame->location.line, frame->location.column);
        }
        length += strlen(line) + 2;
        parts = consList(line, parts);
        list = list->tail;
    }

    const char* header = "Traceback:";
    parts  = consList(newStr(header), parts);
    length += strlen(header) + 2;

    char* joined = (char*) malloc(length + 1);
    joined[0] = 0;
    List* partsOriginal = parts;

    while(parts != NULL) {
        strcat(joined, "\t");
        strcat(joined, (const char*) parts->head);
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
        if(typeOfThing(args[i]) != va_arg(types, ThingTypes)) {
            va_end(types);
            return throwMsg(runtime, formatStr("wrong type for argument %i", i + 1));
        }
    }

    va_end(types);

    return createRetVal(NULL, 0);
}

Thing* createTupleThing(Runtime* runtime, uint8_t size, Thing** elements) {
    return createThing(runtime, new TupleThing(size, elements));
}

uint8_t getTupleSize(Thing* tuple) {
    return ((TupleThing*) tuple)->size;
}

Thing* getTupleElem(Thing* tuple, uint8_t index) {
    return ((TupleThing*) tuple)->elements[index];
}

Thing* createListThing(Runtime* runtime, Thing* head, Thing* tail) {
    return createThing(runtime, new ListThing(head, tail));
}

Thing* createObjectThing(Runtime* runtime, Map* map) {
    return createThing(runtime, new ObjectThing(map));
}

Map* getObjectMap(Thing* object) {
    return ((ObjectThing*) object)->map;
}

void destroyObjectThing(Thing* self) {
}

Thing* createCellThing(Runtime* runtime, Thing* value) {
    return createThing(runtime, new CellThing(value));
}

Thing* getCellValue(Thing* cell) {
    return ((CellThing*) cell)->value;
}

void setCellValue(Thing* cell, Thing* value) {
    ((CellThing*) cell)->value = value;
}
