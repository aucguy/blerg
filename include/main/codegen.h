#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <stdint.h>

#include "main/util.h"
#include "main/bytecode.h"
#include "main/parse.h"

/**
 * Contains an alternative representation of a module as its being built.
 * Useful for building modules.
 */
typedef struct {
    /*
     * Instead of the constants and bytecode being stored as a continuous
     * array, They are instead split into arrays of SEGMENT_SIZE and stored as
     * a list in reverse order. As new elements are added, the first array is
     * filled up. Once it is full, a new array is created and added to the
     * beginning of the list.
     *
     * Note that constantsLength and bytecodeLength count the number of filled
     * elements in all the segments of their respective lists. They do not
     * count the current capacity.
     */

    uint32_t constantsLength;
    List* constants;

    uint32_t bytecodeLength;
    List* bytecode;

    uint32_t srcLocLength;
    List* srcLoc;

    /**
     * Since labels are referenced in instructions before they are defined via
     * emitLabel, the builder needs to remember where labels are
     * referenced and then patch them later when the label definition becomes
     * available. Additionally, labels may be referenced after they are defined
     * so, the builder needs to remember the definition of labels so they can
     * be added later.
     *
     * To simplify things, all label references are emitted as a constant value
     * and are patched in builderToModule, regardless if they happen before or
     * after the label definition. All label references and definitions are
     * stored here.
     */

    //the next label number. It is incremented by one each time a label is created
    //in order to get a unique label.
    uint32_t nextLabel;

    //stores the definition of labels.
    //Maps labels (ints) to bytecode positions (ints)
    Map* labelDefs;

    //stores the references of labels.
    //Maps labels (ints) to lists of bytecode positions (ints)
    Map* labelRefs;
} ModuleBuilder;

ModuleBuilder* createModuleBuilder();
void destroyModuleBuilder(ModuleBuilder* builder);

/**
 * Creates a label. Each label is uniquely referenced by an integer. Different
 * ModuleBuilders may return the same integer value, but a single
 * ModuleBuilder will always return a different value for each call.
 */
uint32_t createLabel(ModuleBuilder* builder);

/**
 * Defines that a label occurs at the current bytecode position. Note that
 * there is no way to define a label to occur at a point that is not the
 * current position.
 */
void emitLabel(ModuleBuilder* builder, uint32_t label);
void emitPushInt(ModuleBuilder* builder, int32_t num);
void emitPushFloat(ModuleBuilder* builder, float num);

/**
 * TODO fix documentation
 * Emits a PUSH_BUILTIN instruction. The function does not hold a reference to
 * symbol. If necessary, it will create a copy to store in the constants table.
 */
void emitPushBuiltin(ModuleBuilder* builder, const char* name);
void emitPushLiteral(ModuleBuilder* builder, const char* literal);
void emitPushNone(ModuleBuilder* builder);
void emitCall(ModuleBuilder* builder, uint32_t arity);
void emitReturn(ModuleBuilder* builder);

/**
 * Emits a CREATE_FUNC instruction. The location is the label just before the
 * function's bytecode.
 */
void emitCreateFunc(ModuleBuilder* builder, uint32_t location);

void emitLoad(ModuleBuilder* builder, const char* name);
void emitStore(ModuleBuilder* builder, const char* name);

/**
 * Emits a conditional jump instruction.
 *
 * @param builder the ModuleBuilder instance
 * @param label the label to jump to if the condition holds
 * @param when if 0 the jump will be taken if the popped value is falsy,
 *      otherwise, the jump will be taken if the popped value is truthy
 */
void emitCondJump(ModuleBuilder* builder, uint32_t label, uint8_t when);

/**
 * Emits an unconditional jump instruction
 *
 * @param builder the ModuleBuilder instance
 * @param label the label to jump to unconditionally
 */
void emitAbsJump(ModuleBuilder* builder, uint32_t label);

/**
 * Emits a function definition instruction.
 *
 * @param builder the ModuleBuilder instance
 * @param argNum the arity of the function. This should be the length of args
 * @param args an array of the argument names. The function will not hold a
 *      reference to the array or any of its elements; it will copy them as
 *      necessary.
 */
void emitDefFunc(ModuleBuilder* builder, uint8_t argNum, const char** args,
        uint8_t isInit);

void emitSrcLoc(ModuleBuilder* builder, SrcLoc location);

/**
 * Turns a ModuleBuilder into a module suitable for interpreting. This compacts
 * the segments into a single array and patches the label references.
 */
Module* builderToModule(ModuleBuilder* builder, uint32_t entry);

/**
 * Takes the AST and turns into a compiled Module object.
 */
Module* compileModule(Token* ast);
void destroyModule(Module* module);

#endif /* CODEGEN_H_ */
