#include "main/util.h"
#include "main/parse.h"

#ifndef BYTECODE_H_
#define BYTECODE_H_

enum INSTRUCTIONS {
    //args: value (int)
    //stack: -> int
    //reads an integer from the bytecode and pushes its value onto the stack
    OP_PUSH_INT,
    //args: index (int)
    //stack: -> symbol
    //reads an integer from the bytecode, looks it up in the constant table,
    //and pushes the symbol onto the stack
    OP_PUSH_SYMBOL,
    //args:
    //stack: -> None
    //pushes the None object onto the stack
    OP_PUSH_NONE,
    //args:
    //stack: f x -> y
    //pops x then f off the stack, calls f with x and then pushes the returned
    //value onto the stack.
    OP_CALL,
    //args:
    //stack: x ->
    //pops x off the stack and returns x. This is the value that is pushed onto
    //the stack in the OP_CALL instruction of the caller.
    OP_RETURN,
    //args: label (int)
    //stack: -> func
    //creates a new function which inherits the current scope is the current
    //scope and whose entry point is the label.
    OP_CREATE_FUNC,
    //args: name (int)
    //stack: value -> obj
    //Looks up the name in the constant table. The instruction then assigns the
    //local variable with that name within the local scope.
    OP_STORE,
    //This opcode performs no operations, but denotes the beginning of a
    //function. It is used to tell the runtime function object the function's
    //arity and the names to bind the arguments to. The format is
    //
    //<byte opcode = OP_DEF_FUNC> <int argNum> (<int argNameIndex>)+
    //
    //The opcode is followed by an integer operand that denotes the function's
    //arity. Following the arity is a variable number of integer operands whose
    //values correspond to the index of the argument names in the constant
    //table. The number of these arguments is equal to the functions arity
    //found in the first operand.
    //
    //For example, "def myFunc x y then ..." would become "OP_DEF_FUNC 2 35 36"
    //assuming that 35 corresponds to "x" and 36 corresponds to "y" in the
    //constant table.
    OP_DEF_FUNC
};

/**
 * The compiled contents of a module. This contains none of the values
 * generated at runtime.
 */
typedef struct {
    //the constant table, constantsLength is the length of constants
    unsigned int constantsLength;
    const char** constants;

    //the bytecode, bytecodeLength is the length of bytecode
    unsigned int bytecodeLength;
    const unsigned char* bytecode;
} Module;

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

    unsigned int constantsLength;
    List* constants;

    unsigned int bytecodeLength;
    List* bytecode;

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
    int nextLabel;

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
int createLabel(ModuleBuilder* builder);

/**
 * Defines that a label occurs at the current bytecode position. Note that
 * there is no way to define a label to occur at a point that is not the
 * current position.
 */
void emitLabel(ModuleBuilder* builder, int label);
void emitPushInt(ModuleBuilder* builder, int num);

/**
 * Emits a PUSH_SYMBOL instruction. The function does not hold a reference to
 * symbol. If necessary, it will create a copy to store in the constants table.
 */
void emitPushSymbol(ModuleBuilder* builder, const char* symbol);
void emitPushNone(ModuleBuilder* builder);
void emitCall(ModuleBuilder* builder);
void emitReturn(ModuleBuilder* builder);

/**
 * Emits a CREATE_FUNC instruction. The location is the label just before the
 * function's bytecode.
 */
void emitCreateFunc(ModuleBuilder* builder, int location);
void emitStore(ModuleBuilder* builder, const char* name);

/**
 * Emits a function definition instruction.
 *
 * @param builder the ModuleBuilder instance
 * @param argNum the arity of the function. This should be the length of args
 * @param args an array of the argument names. The function will not hold a
 *      reference to the array or any of its elements; it will copy them as
 *      necessary.
 */
void emitDefFunc(ModuleBuilder* builder, char argNum, const char** args);

/**
 * Turns a ModuleBuilder into a module suitable for interpreting. This compacts
 * the segments into a single array and patches the label references.
 */
Module* builderToModule(ModuleBuilder* builder);

/**
 * Takes the AST and turns into a compiled Module object.
 */
Module* compileModule(Token* ast);
void destroyModule(Module* module);

#endif /* BYTECODE_H_ */
