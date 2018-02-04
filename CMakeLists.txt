cmake_minimum_required(VERSION 3.9)
project(lavascript)

set(CMAKE_CXX_STANDARD 17)

add_executable(lavascript
        generated/local/FlexLexerLocal.h
        generated/local/lexer.cpp
        generated/local/parser.cpp
        generated/local/parser.hpp
        generated/local/stack.hh
        generated/lexer.cpp
        generated/parser.cpp
        generated/parser.hpp
        generated/stack.hh
        source/cli/ls_main.cpp
        source/lib/ls_build_config.hpp
        source/lib/ls_builtins.cpp
        source/lib/ls_common.cpp
        source/lib/ls_compiler.cpp
        source/lib/ls_opcodes.hpp
        source/lib/ls_operators.cpp
        source/lib/ls_output.cpp
        source/lib/ls_pool.hpp
        source/lib/ls_registry.hpp
        source/lib/ls_runtime.cpp
        source/lib/ls_semantic_check.cpp
        source/lib/ls_symbol_table.cpp
        source/lib/ls_syntax_tree.cpp
        source/lib/ls_vm.cpp
        source/lib/ls_vm.hpp
        tests/cpp/ls_run_samples.cpp
        tests/cpp/ls_script_call.cpp
        tests/cpp/ls_script_globals.cpp)