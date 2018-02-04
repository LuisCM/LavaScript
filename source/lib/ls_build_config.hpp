// ================================================================================================
// -*- C++ -*-
// File: ls_build_config.hpp
// Author: LuisCM
// Created on: 24/08/2001
// Brief: Customizable preprocessors switches. Reasonable defaults provided for each.
// ================================================================================================

#ifndef LAVASCRIPT_BUILD_CONFIG_HPP
#define LAVASCRIPT_BUILD_CONFIG_HPP

// Necessary for the PRIi64/PRIX64 below.
#include <cinttypes>

// NOTE:
// Pools sizes are in items/elements, not bytes!

//
// Size in items of each memory block allocated by the intermediate instruction
// pool used by the Compiler. Larger values reduce the number of allocations.
//
#ifndef LAVASCRIPT_INTERMEDIATE_INSTR_POOL_GRANULARITY
    #define LAVASCRIPT_INTERMEDIATE_INSTR_POOL_GRANULARITY 1024
#endif // LAVASCRIPT_INTERMEDIATE_INSTR_POOL_GRANULARITY

//
// Size of each memory block allocated by the TypeTable TypeId pool.
// This doesn't have to be very big. New TypeIds are only added for
// user-defined structs, enums and type-aliases.
//
#ifndef LAVASCRIPT_TYPEID_POOL_GRANULARITY
    #define LAVASCRIPT_TYPEID_POOL_GRANULARITY 256
#endif // LAVASCRIPT_TYPEID_POOL_GRANULARITY

//
// Size of the SymbolTable pool. Each identifier and literal constant
// becomes a symbol (only once for every occurrence of each), so this
// pool can get large.
//
#ifndef LAVASCRIPT_SYMBOL_POOL_GRANULARITY
    #define LAVASCRIPT_SYMBOL_POOL_GRANULARITY 512
#endif // LAVASCRIPT_SYMBOL_POOL_GRANULARITY

//
// Pool of SyntaxTreeNodes. Parsing the source code will generate a lot of
// nodes, so a larger value for the blocks in this pool is recommended.
//
#ifndef LAVASCRIPT_AST_NODE_POOL_GRANULARITY
    #define LAVASCRIPT_AST_NODE_POOL_GRANULARITY 512
#endif // LAVASCRIPT_AST_NODE_POOL_GRANULARITY

//
// Block size of the Object pools used by the Garbage Collector.
//
#ifndef LAVASCRIPT_RT_OBJECT_POOL_GRANULARITY
    #define LAVASCRIPT_RT_OBJECT_POOL_GRANULARITY 256
#endif // LAVASCRIPT_RT_OBJECT_POOL_GRANULARITY

//
// If this is enabled, the VM class will have a GlobalsTable member
// that allows accessing all the program globals by name. Functions
// are also considered globals, so they can be accessed via the
// globals table too.
//
#ifndef LAVASCRIPT_GLOBALS_TABLE
    #define LAVASCRIPT_GLOBALS_TABLE 1
#endif // LAVASCRIPT_GLOBALS_TABLE

//
// If this is enabled, the VM class will have a printStackTrace()
// method that you can call to dump the current script call stack,
// including native calls. This functionality requires an additional
// stack to keep track of the functions names, so you might wish to
// disable this to save memory in "release" mode.
//
#ifndef LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
    #define LAVASCRIPT_SAVE_SCRIPT_CALLSTACK 1
#endif // LAVASCRIPT_SAVE_SCRIPT_CALLSTACK

//
// When defined to nonzero, constructor calls are allowed to not
// initialize all members. The uninitialized members will be set
// to defaults. If not defined or if = 0, not providing all the
// initializers is a runtime error.
//
#ifndef LAVASCRIPT_ALLOW_UNINITIALIZED_MEMBERS
    #define LAVASCRIPT_ALLOW_UNINITIALIZED_MEMBERS 1
#endif // LAVASCRIPT_ALLOW_UNINITIALIZED_MEMBERS

//
// Print a list of runtime flags when dumping the CG objects.
//
#ifndef LAVASCRIPT_PRINT_RT_OBJECT_FLAGS
    #define LAVASCRIPT_PRINT_RT_OBJECT_FLAGS 1
#endif // LAVASCRIPT_PRINT_RT_OBJECT_FLAGS

//
// This debug flag enables several initialization tests and runtime checks.
// It is recommended to leave this on during development, but you can you
// turn it off to maximize performance on a "release" build.
//
#ifndef LAVASCRIPT_DEBUG
    #if defined(DEBUG) || defined(_DEBUG)
        #define LAVASCRIPT_DEBUG 1
    #endif // DEBUG || _DEBUG
#endif // LAVASCRIPT_DEBUG

//
// Runtime assertions are enabled by default if building in debug mode.
// They can be left on or disabled regardless of the debug preset.
//
#ifndef LAVASCRIPT_ENABLE_ASSERT
    #if LAVASCRIPT_DEBUG
        #define LAVASCRIPT_ENABLE_ASSERT 1
    #endif // LAVASCRIPT_DEBUG
#endif // LAVASCRIPT_ENABLE_ASSERT

//
// When this is defined to nonzero, error and warning messages will
// be printed using ANSI color code prefixes, so that they display
// with colored text in a console/terminal window. If stdout or stderr
// are redirected, then we will always refuse to print using color codes,
// even if this macro was defined to true.
//
// Note:
// ANSI color codes are not supported on the standard Windows console (cmd.exe)
//
#ifndef LAVASCRIPT_PRINT_USE_ANSI_COLOR_CODES
    #if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
        #define LAVASCRIPT_PRINT_USE_ANSI_COLOR_CODES 1
    #endif // Apple, Linux, Unix
#endif // LAVASCRIPT_PRINT_USE_ANSI_COLOR_CODES

//
// Portability macros for printf-like functions printing 64bits integers:
//
#ifndef LAVASCRIPT_I64_PRINT_FMT
    #ifdef PRIi64
        #define LAVASCRIPT_I64_PRINT_FMT "%" PRIi64
    #else // PRIi64 not defined
        #define LAVASCRIPT_I64_PRINT_FMT "%lli"
    #endif // PRIi64
#endif // LAVASCRIPT_I64_PRINT_FMT

#ifndef LAVASCRIPT_X64_PRINT_FMT
    #ifdef PRIX64
        #define LAVASCRIPT_X64_PRINT_FMT "0x%016" PRIX64
    #else // PRIX64 not defined
        #define LAVASCRIPT_X64_PRINT_FMT "0x%016llX"
    #endif // PRIX64
#endif // LAVASCRIPT_X64_PRINT_FMT

#endif // LAVASCRIPT_BUILD_CONFIG_HPP
