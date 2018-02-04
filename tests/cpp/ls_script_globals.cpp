// ================================================================================================
// -*- C++ -*-
// File: ls_script_globals.cpp
// Author: LuisCM
// Created on: 24/08/2001
// Brief: Sample showing how to manipulate global script variables from C++.
// ================================================================================================

#include "ls_compiler.hpp"
#include "ls_vm.hpp"

#if !defined(LAVASCRIPT_GLOBALS_TABLE) || (LAVASCRIPT_GLOBALS_TABLE == 0)
    #error "This sample requires LAVASCRIPT_GLOBALS_TABLE to be defined!"
#endif

// -----
// main
// -----
int main()
{
    using namespace lavascript;

    LSCompiler compiler; // Script parsing and bytecode output.
    LSVM       vm;       // Executes bytecode produced by a Compiler.

    try
    {
        compiler.parseScript(&vm, "tests/script/globals.ls");
        compiler.compile(&vm);

        // Init the globals:
        logStream() << "C++: First run...\n";
        vm.execute();

        // Print the current values:
        logStream() << "C++: Calling printGlobals()...\n";
        vm.call("printGlobals");

        // Change them:
        logStream() << "C++: Editing globals vars...\n";
        {
            Variant var;

            var.type = Variant::Type::Integer;
            var.value.asInteger = 1337;
            LAVASCRIPT_ASSERT(vm.globals.setGlobal("an_integer", var) == true);

            var.type = Variant::Type::Float;
            var.value.asFloat = 3.141592;
            LAVASCRIPT_ASSERT(vm.globals.setGlobal("an_float", var) == true);

            var.type = Variant::Type::Range;
            var.value.asRange.begin = -5;
            var.value.asRange.end   = +5;
            LAVASCRIPT_ASSERT(vm.globals.setGlobal("an_range", var) == true);

            var.type = Variant::Type::Str;
            const char * s = "Hello from C++";
            var.value.asString = LSStr::newFromString(vm, s, std::strlen(s), true);
            LAVASCRIPT_ASSERT(vm.globals.setGlobal("an_string", var) == true);
        }

        // Print the new values:
        logStream() << "C++: Calling printGlobals() again...\n";
        vm.call("printGlobals");
    }
    catch (...)
    {
        #if LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
        if (!vm.callstack.isEmpty())
        {
            vm.printStackTrace(logStream());
        }
        #endif // LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
        logStream() << color::red() << "terminating with error(s)...\n" << color::restore();
        return EXIT_FAILURE;
    }
}
