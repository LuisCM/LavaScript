// ================================================================================================
// -*- C++ -*-
// File: ls_output.cpp
// Author: LuisCM
// Created on: 01/01/2001
// Brief: Runtime output printing helpers.
// ================================================================================================

#include "ls_output.hpp"
#include "ls_common.hpp"
#include <cstdlib> // std::abort

namespace lavascript
{

// ========================================================
// LavaScript compiler and runtime output streams:
// ========================================================

static std::ostream * g_outStr = &std::cout;
static std::ostream * g_errStr = &std::cerr;

std::ostream & logStream() noexcept { return *g_outStr; }
std::ostream & errStream() noexcept { return *g_errStr; }

void setStreams(std::ostream * outStr, std::ostream * errStr) noexcept
{
    g_outStr = outStr;
    g_errStr = errStr;
    if (g_outStr == nullptr) { g_outStr = &std::cout; }
    if (g_errStr == nullptr) { g_errStr = &std::cerr; }
}

// ========================================================
// Compiler/runtime output and error reporting:
// ========================================================

static void formatOffendingCode(std::string & message, const std::string * offendingCode)
{
    if (offendingCode == nullptr || offendingCode->empty())
    {
        return;
    }

    message += "\n";
    message += color::cyan();
    message += *offendingCode;

    if (message.back() == '\n')
    {
        // No newlines before the '<---'
        message.pop_back();
    }

    message += color::restore();
    message += " <---\n";
}

void warning(const std::string & warnMessage,
             const std::string & srcFile,
             const int srcLineNum,
             const std::string * offendingCode)
{
    const std::string warningTag = color::magenta() + toString("warning:") + color::restore();
    std::string message = srcFile + "(" + toString(srcLineNum) + "): " + warningTag + " " + warnMessage;
    formatOffendingCode(message, offendingCode);

    errStream() << message << "\n";
}

[[noreturn]] void scriptError(const std::string & errorMessage) noexcept(false)
{
    errStream() << errorMessage << std::endl;
    throw ScriptException{ errorMessage };
}

[[noreturn]] void lexerError(const std::string & errorMessage,
                             const std::string & srcFile,
                             const int srcLineNum,
                             const std::string * offendingCode) noexcept(false)
{
    const std::string errorTag = color::red() + toString("lex error:") + color::restore();
    std::string message = srcFile + "(" + toString(srcLineNum) + "): " + errorTag + " " + errorMessage;
    formatOffendingCode(message, offendingCode);

    errStream() << message << std::endl;
    throw LexerException{ "Lexer error" };
}

[[noreturn]] void parserError(const std::string & errorMessage,
                              const std::string & srcFile,
                              const int srcLineNum,
                              const std::string * offendingCode) noexcept(false)
{
    const std::string errorTag = color::red() + toString("parse error:") + color::restore();
    std::string message = srcFile + "(" + toString(srcLineNum) + "): " + errorTag + " " + errorMessage;
    formatOffendingCode(message, offendingCode);

    errStream() << message << std::endl;
    throw ParserException{ "Parser error" };
}

[[noreturn]] void runtimeError(const std::string & errorMessage,
                               const std::string & srcFile,
                               const int srcLineNum) noexcept(false)
{
    const std::string errorTag = color::red() + toString("runtime error:") + color::restore();
    const std::string message  = srcFile + "(" + toString(srcLineNum) + "): " + errorTag + " " + errorMessage;

    errStream() << message << std::endl;
    throw RuntimeException{ "Runtime error" };
}

[[noreturn]] void internalError(const std::string & errorMessage,
                                const std::string & srcFile,
                                const int srcLineNum) noexcept(false)
{
    const std::string errorTag = color::red() + toString("internal compiler error:") + color::restore();
    const std::string message  = srcFile + "(" + toString(srcLineNum) + "): " + errorTag + " " + errorMessage;

    errStream() << message << std::endl;
    throw CompilerException{ "Internal compiler error" };
}

[[noreturn]] void runtimeAssertionError(const std::string & errorMessage,
                                        const std::string & srcFile,
                                        const int srcLineNum)
{
    const std::string errorTag = color::yellow() + toString("runtime assertion failed:") + color::restore();
    const std::string message  = srcFile + "(" + toString(srcLineNum) + "): " + errorTag + " " + errorMessage;

    errStream() << message << std::endl;
    std::abort();
}

[[noreturn]] void unreachable(const std::string & srcFile, const int lineNum)
{
    runtimeAssertionError("trying to run unreachable code path!", srcFile, lineNum);
}

// ========================================================

#if LAVASCRIPT_DEBUG
namespace
{
struct LogTest
{
    LogTest()
    {
        lavascript::logStream() << "LavaScript: This is a debug build. Compile without LAVASCRIPT_DEBUG to silence these messages.\n";
        #if LAVASCRIPT_ENABLE_ASSERT
        lavascript::logStream() << "LavaScript: Runtime asserts are enabled.\n";
        #endif // LAVASCRIPT_ENABLE_ASSERT
    }
} localLogTest;
} // namespace {}
#endif // LAVASCRIPT_DEBUG

} // namespace lavascript {}
