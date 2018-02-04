// ==========================================================================================
// -*- C++ -*-
// File: ls_run_samples.cpp
// Author: LuisCM
// Created on: 24/08/2001
// Brief: This test spins up a Compiler and VM for each of the sample scripts and runs them.
// ==========================================================================================

#include "ls_compiler.hpp"
#include "ls_vm.hpp"

// ----------
// runScript
// ----------
static void runScript(const std::string &scriptFile)
{
    using namespace lavascript;

    LSCompiler compiler; // Script parsing and bytecode output.
    LSVM       vm;       // Executes bytecode produced by a Compiler.

    try
    {
        compiler.parseScript(&vm, scriptFile);
        compiler.compile(&vm);
        vm.execute();
    }
    catch (...)
    {
        #if LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
        if (!vm.callstack.isEmpty())
        {
            vm.printStackTrace(logStream());
        }
        #endif // LAVASCRIPT_SAVE_SCRIPT_CALLSTACK

        logStream() << color::red() << "terminating script \"" << scriptFile
                    << "\" with error(s).\n" << color::restore();
    }
}

// -----
// main
// -----
int main()
{
    const std::string path = "tests/script/";
    const std::string scripts[]
    {
        path + "callstack.ls",
        path + "cpp_call.ls",
        path + "enums.ls",
        path + "fibonacci.ls",
        path + "functions.ls",
        path + "gc.ls",
        path + "globals.ls",
        path + "if-else.ls",
        path + "imports.ls",
        path + "loops.ls",
        path + "match.ls",
        path + "native_types.ls",
        path + "structs.ls",
        path + "test-import-1.ls",
        path + "test-import-2.ls",
        path + "unused-expr.ls",
    };
    for (const auto &s : scripts)
    {
        runScript(s);
    }
}
