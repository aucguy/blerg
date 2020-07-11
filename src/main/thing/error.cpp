#include <stdlib.h>
#include <string.h>

#include "main/runtime.h"
#include "main/thing.h"
#include "main/util.h"
#include "main/thing/error.h"

ErrorThing::ErrorThing(const char* msg, List* stackFrame) :
    msg(msg), stackFrame(stackFrame) {}

ErrorThing::~ErrorThing() {
    free((char*) this->msg);
    destroyList(this->stackFrame, free);
}

RetVal ErrorThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal ErrorThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return symbolDispatch(runtime, self, args, arity);
}

ThingType ErrorThing::type() {
    return TYPE_ERROR;
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
    //UNUSED(runtime);

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
