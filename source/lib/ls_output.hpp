// ================================================================================================
// -*- C++ -*-
// File: ls_output.hpp
// Author: LuisCM
// Created on: 24/08/2001
// Brief: Runtime output printing helpers.
// ================================================================================================

#ifndef LAVASCRIPT_OUTPUT_HPP
#define LAVASCRIPT_OUTPUT_HPP

#include <string>
#include <cstdio>
#include <ostream>
#include "ls_build_config.hpp"

// ========================================================
// isatty() only needed if colored text output is desired.
// ========================================================

#if LAVASCRIPT_PRINT_USE_ANSI_COLOR_CODES
    // For isatty()/fileno()
    #if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
        #include <unistd.h>
    #elif defined(_WIN32) || defined(_MSC_VER)
        #include <io.h>
        // Damn Windows with your silly underscores...
        #ifndef isatty
            #define isatty _isatty
        #endif // isatty
        #ifndef fileno
            #define fileno _fileno
        #endif // fileno
    #endif // Apple/Win/Linux
#endif // LAVASCRIPT_PRINT_USE_ANSI_COLOR_CODES

namespace lavascript
{

// ========================================================
// Colored text printing on the terminal:
// ========================================================

namespace color
{

#if LAVASCRIPT_PRINT_USE_ANSI_COLOR_CODES
    inline bool colorPrintEnabled() noexcept
    {
        // Only attempt color print if none of the streams we use were redirected.
        // We can also cache the value since it is unlikely that this will change
        // while the program is still running.
        static bool notRedirected = (isatty(fileno(stdout)) && isatty(fileno(stderr)));
        return notRedirected;
    }
#else // !LAVASCRIPT_PRINT_USE_ANSI_COLOR_CODES
    constexpr bool colorPrintEnabled() noexcept { return false; }
#endif // LAVASCRIPT_PRINT_USE_ANSI_COLOR_CODES

// ANSI color codes:
inline const char * restore() noexcept { return colorPrintEnabled() ? "\033[0;1m"  : ""; }
inline const char * red()     noexcept { return colorPrintEnabled() ? "\033[31;1m" : ""; }
inline const char * green()   noexcept { return colorPrintEnabled() ? "\033[32;1m" : ""; }
inline const char * yellow()  noexcept { return colorPrintEnabled() ? "\033[33;1m" : ""; }
inline const char * blue()    noexcept { return colorPrintEnabled() ? "\033[34;1m" : ""; }
inline const char * magenta() noexcept { return colorPrintEnabled() ? "\033[35;1m" : ""; }
inline const char * cyan()    noexcept { return colorPrintEnabled() ? "\033[36;1m" : ""; }
inline const char * white()   noexcept { return colorPrintEnabled() ? "\033[37;1m" : ""; }

} // namespace color {}

// ========================================================
// Error and log output streams:
// ========================================================

std::ostream & logStream() noexcept; // std::cout by default
std::ostream & errStream() noexcept; // std::cerr by default
void setStreams(std::ostream * outStr, std::ostream * errStr) noexcept;

// ========================================================
// Compiler/runtime output and error reporting:
// ========================================================

// Use this for non-fatal Lexer/Parser/Compiler warning messages.
// Justs logs to the error output (stderr/std::cerr by default). Doesn't explicitly throw.
void warning(const std::string & warnMessage,
             const std::string & srcFile,
             const int srcLineNum,
             const std::string * offendingCode = nullptr);

// Use this for the native script error handlers (script panic/assert/etc).
// The input message should already carry the script source location info.
// Logs to the error output (stderr/std::cerr by default) and throws
// ScriptException (with the error message).
[[noreturn]] void scriptError(const std::string & errorMessage) noexcept(false);

// Use this for Lexer error output only. Logs to stderr and throws
// LexerException. Source location should come from the offending script.
[[noreturn]] void lexerError(const std::string & errorMessage,
                             const std::string & srcFile,
                             const int srcLineNum,
                             const std::string * offendingCode = nullptr) noexcept(false);

// Use this for Parser error output only. Logs to stderr and throws
// ParserException. Source location should come from the offending script.
[[noreturn]] void parserError(const std::string & errorMessage,
                              const std::string & srcFile,
                              const int srcLineNum,
                              const std::string * offendingCode = nullptr) noexcept(false);

// Use this to report fatal runtime/VM errors on the C++ side.
// Logs to stderr and throws RuntimeException.
[[noreturn]] void runtimeError(const std::string & errorMessage,
                               const std::string & srcFile,
                               const int srcLineNum) noexcept(false);

// Use this to halt on internal errors or failed preconditions on the C++ side.
// Logs to stderr and throws CompilerException.
[[noreturn]] void internalError(const std::string & errorMessage,
                                const std::string & srcFile,
                                const int srcLineNum) noexcept(false);

// Use this to report assertion failures on the C++ side.
// Logs to stderr and calls std::abort().
[[noreturn]] void runtimeAssertionError(const std::string & errorMessage,
                                        const std::string & srcFile,
                                        const int srcLineNum);

// Call this to please then compiler when we have unreachable code in
// switch statements and the like. Logs to stderr and calls std::abort().
[[noreturn]] void unreachable(const std::string & srcFile,
                              const int lineNum);

// ========================================================
// Helper macros:
// ========================================================

//
// Convenience helpers that adds the source location info for us:
//
#define LAVASCRIPT_UNREACHABLE()               ::lavascript::unreachable(__FILE__, __LINE__)
#define LAVASCRIPT_RUNTIME_EXCEPTION(message)  ::lavascript::runtimeError((message), __FILE__, __LINE__)
#define LAVASCRIPT_INTERNAL_EXCEPTION(message) ::lavascript::internalError((message), __FILE__, __LINE__)

//
// Replacement for standard assert():
//
#if LAVASCRIPT_ENABLE_ASSERT
    #define LAVASCRIPT_ASSERT(expr)                                             \
        do                                                                \
        {                                                                 \
            if (!(expr))                                                  \
            {                                                             \
                ::lavascript::runtimeAssertionError(#expr, __FILE__, __LINE__); \
            }                                                             \
        } while (0)
#else // !LAVASCRIPT_ENABLE_ASSERT
    #define LAVASCRIPT_ASSERT(expr) /* no-op */
#endif // LAVASCRIPT_ENABLE_ASSERT

} // namespace lavascript {}

#endif // LAVASCRIPT_OUTPUT_HPP
