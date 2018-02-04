// ================================================================================================
// -*- C++ -*-
// File: ls_compiler.hpp
// Author: LuisCM
// Created on: 24/08/2001
// Brief: The Compiler class transforms a Syntax Tree representation into VM bytecode.
// ================================================================================================

#ifndef LAVASCRIPT_COMPILER_HPP
#define LAVASCRIPT_COMPILER_HPP

#include "ls_vm.hpp"
#include "ls_symbol_table.hpp"
#include "ls_syntax_tree.hpp"
#include "ls_semantic_check.hpp"

namespace lavascript
{

// ========================================================
// struct IntermediateInstr:
// ========================================================

struct IntermediateInstr final
{
    union Operand
    {
        const Symbol            * symbol;
        const IntermediateInstr * jumpTarget;
    };

    IntermediateInstr * next;
    Operand             operand;
    UInt32              uid;
    UInt16              stackIndex;
    Variant::Type       dataType;
    OpCode              op;
};

constexpr UInt16 InvalidStackIndex = UInt16(~0);
using InstrPool = LSPool<IntermediateInstr, LAVASCRIPT_INTERMEDIATE_INSTR_POOL_GRANULARITY>;

// ========================================================
// class FileIOCallbacks:
// ========================================================

class LSFileIOCallbacks
{
public:

    virtual ~LSFileIOCallbacks();

    // Called to open a file for an explicit LSCompiler::parseScript() call.
    // Should return true only if the file was successfully opened for reading.
    virtual bool openScript(const std::string & scriptFile, std::istream ** streamOut) = 0;

    // Called when an 'import' directive is encountered in a script.
    // Receives the unaltered name string that followed the keyword.
    // The implementation is free to perform any path-dependent file
    // lookup or append predefined search paths to the name.
    virtual bool openScriptImport(const std::string & importFile, std::istream ** streamOut) = 0;

    // Called to return a stream object previously acquired by the above methods.
    // NOTE: If the open method returned false, closeScript() is not called!
    virtual void closeScript(std::istream ** stream) = 0;
};

// ========================================================
// class LSCompiler:
// ========================================================

// The Compiler performs parsing and VM bytecode generation.
// We use a three stage compilation process. First, a syntax tree
// is built from the source code, then a list of Intermediate
// Instructions is built from the tree. Finally, the Intermediate
// Instructions are compiled into runnable VM bytecode.
//
// Each processed script will spin up a Lexer and a Parser instance.
// You can parse any number of scripts before calling compile()
// to generation the final VM bytecode program.
//
// Providing a set of custom IO callbacks allows for different
// script file lookup strategies, such as loading from compressed
// directories and archives.
class LSCompiler final
{
public:

    //
    // Public compiler interface:
    //

    LSCompiler();
    explicit LSCompiler(LSFileIOCallbacks * newIOCallbacks);

    // Not copyable.
    LSCompiler(const LSCompiler &) = delete;
    LSCompiler & operator = (const LSCompiler &) = delete;

    // Opens the given file to parse the script source code and generate a syntax tree
    // from it. This will also perform semantic validation in the source and throw parse
    // errors if there are any. Bytecode is only generated later when compile() is called.
    // Functions and types will be registered with the given VM instance. You should pass
    // the same pointer again when calling compile().
    void parseScript(LSVM * vm, const std::string & filename,
                     SyntaxTreeNode ** outTreeRoot = nullptr);

    // Same as the above, but takes an already opened file stream,
    // so the LSFileIOCallbacks are not used.
    void parseScript(LSVM * vm, std::istream * source,
                     const std::string & filename,
                     SyntaxTreeNode ** outTreeRoot = nullptr);

    // This one opens the script using LSFileIOCallbacks::openScriptImport(), so it is
    // meant for internal parser use to handle the import directives in the scripts.
    void parseScriptImport(LSVM * vm, const std::string & filename,
                           SyntaxTreeNode ** outImportRoot = nullptr);

    // Runs the compilation process to produce bytecode and program data from the syntax tree built
    // during parsing, which is then ready to be run on a LavaScript VM instance. Might throw compiler errors.
    void compile(LSVM * vm);

    // Set the LSFileIOCallbacks used by this compiler.
    // The IO callbacks are used to open the script file on parseScript() and
    // also to open subsequent imports found while parsing the initial script.
    // By default the compiler will use the Standard <fstream> C++ API to open files.
    // Passing null to set() will restore the default callbacks.
    void setFileIOCallbacks(LSFileIOCallbacks * newIOCallbacks) noexcept;

    // Debug printing for the generated intermediate code.
    void print(std::ostream & os) const;

    //
    // Compiler internals:
    //

    LSSymbolTable         symTable;
    LSSyntaxTree          syntTree;
    LSVarInfoTable        varInfo;
    LSImportTable         importTable;
    InstrPool             instrPool;
    UInt32                instrCount       = 0;
    IntermediateInstr   * globCodeListHead = nullptr;
    IntermediateInstr   * funcCodeListHead = nullptr;
    LSFileIOCallbacks   * fileIOCallbacks  = nullptr;
    bool                  debugMode        = true;
    bool                  enableWarnings   = true;
};

inline std::ostream & operator << (std::ostream & os, const LSCompiler & compiler)
{
    // Prints the globCodeListHead and funcCodeListHead.
    compiler.print(os);
    return os;
}

} // namespace lavascript {}

#endif // LAVASCRIPT_COMPILER_HPP
