// ================================================================================================
// -*- C++ -*-
// File: ls_main.cpp
// Author: LuisCM
// Created on: 24/08/2001
// Brief: Command Line Interpreter (CLI) entry point.
// ================================================================================================

#include "ls_compiler.hpp"
#include "ls_vm.hpp"

using namespace lavascript;

// -------------
// CmdLineFlags
// -------------
struct CmdLineFlags final {
	// Interpreter flags:
	bool run = true;  // <none>
	bool parse = true;  // -P
	bool compile = true;  // -C
	bool debug = false; // -D
	bool warnings = false; // -W
	bool verbose = false; // -V

	// Debug visualization flags:
	bool dumpSymbols = false; // -dump=symbols
	bool dumpSyntax = false; // -dump=syntax
	bool dumpFuncs = false; // -dump=funcs
	bool dumpTypes = false; // -dump=types
	bool dumpGCObjs = false; // -dump=gc
	bool dumpIntermediateCode = false; // -dump=intrcode
	bool dumpVMBytecode = false; // -dump=bytecode
	bool dumpGlobalData = false; // -dump=globdata

	// ------
	// print
	// ------
	void print(std::ostream & os) const {
		os << "LavaScript: Command line flags:\n";
		if (parse && !run) {
			os << "-P\n";
		}
		if (compile && !run) {
			os << "-C\n";
		}
		if (debug) {
			os << "-D\n";
		}
		if (warnings) {
			os << "-W\n";
		}
		if (verbose) {
			os << "-V\n";
		}
		if (dumpSymbols) {
			os << "-dump=symbols\n";
		}
		if (dumpSyntax) {
			os << "-dump=syntax\n";
		}
		if (dumpFuncs) {
			os << "-dump=funcs\n";
		}
		if (dumpTypes) {
			os << "-dump=types\n";
		}
		if (dumpGCObjs) {
			os << "-dump=gc\n";
		}
		if (dumpIntermediateCode) {
			os << "-dump=intercode\n";
		}
		if (dumpVMBytecode) {
			os << "-dump=bytecode\n";
		}
		if (dumpGlobalData) {
			os << "-dump=globdata\n";
		}
		os << "\n";
	}
};

// -----------
// printUsage
// -----------
static void printUsage(const char * const progName, std::ostream & os) {
	os << "usage:\n" << " $ " << progName << " <script> [options]\n"
			<< " options:\n"
			<< "  -C  Compile but don't run. Useful to perform syntax validation on a script.\n"
			<< "  -P  Stop at the parse stage (syntax tree generated and most of the syntax validation done).\n"
			<< "  -D  Compile and run in debug mode.\n"
			<< "  -W  Enable compiler warnings. Warnings are disabled by default.\n"
			<< "  -V  Run the CLI in verbose mode.\n"
			<< "  -dump=[cmd] where 'cmd' is one of:\n"
			<< "    symbols     Dump the symbol table generated during compilation.\n"
			<< "    syntax      Dump the syntax tree generated during compilation.\n"
			<< "    funcs       Dump a list with all the parsed functions.\n"
			<< "    types       Dump a list with all the parsed types plus built-ins.\n"
			<< "    gc          Dump the list of alive CG objects at the end of a program.\n"
			<< "    intercode   Dump the intermediate code generated during compilation.\n"
			<< "    bytecode    Dump the final VM bytecode.\n"
			<< "    globaldata  Dump the global program memory.\n" << "\n";
}

// -------------
// CmdLineFlags
// -------------
static CmdLineFlags scanCmdLineFlags(const int argc, const char * argv[]) {
	CmdLineFlags flags;
	// Skip the first entry (prog name).
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] != '-') {
			continue;
		}
		if (std::strcmp(argv[i], "-C") == 0) {
			flags.parse = true;
			flags.compile = true;
			flags.run = false;
		} else if (std::strcmp(argv[i], "-P") == 0) {
			flags.parse = true;
			flags.compile = false;
			flags.run = false;
		} else if (std::strcmp(argv[i], "-D") == 0) {
			flags.debug = true;
		} else if (std::strcmp(argv[i], "-W") == 0) {
			flags.warnings = true;
		} else if (std::strcmp(argv[i], "-V") == 0) {
			flags.verbose = true;
		} else if (std::strncmp(argv[i], "-dump", 5) == 0) {
			const char * what = argv[i] + 5;
			if (*what != '=') {
				logStream()
						<< "invalid command line argument: expected '=' after '-dump' flag\n";
				continue;
			}
			++what;
			if (std::strcmp(what, "symbols") == 0) {
				flags.dumpSymbols = true;
			} else if (std::strcmp(what, "syntax") == 0) {
				flags.dumpSyntax = true;
			} else if (std::strcmp(what, "funcs") == 0) {
				flags.dumpFuncs = true;
			} else if (std::strcmp(what, "types") == 0) {
				flags.dumpTypes = true;
			} else if (std::strcmp(what, "gc") == 0) {
				flags.dumpGCObjs = true;
			} else if (std::strcmp(what, "intercode") == 0) {
				flags.dumpIntermediateCode = true;
			} else if (std::strcmp(what, "bytecode") == 0) {
				flags.dumpVMBytecode = true;
			} else if (std::strcmp(what, "globaldata") == 0) {
				flags.dumpGlobalData = true;
			} else {
				if (*what != '\0') {
					logStream() << "unknown '-dump=' flag: " << what << "\n";
				}
			}
		}
	}
	return flags;
}

// -------
// vPrint
// -------
static void vPrint(const CmdLineFlags & flags, const std::string & message) {
	if (flags.verbose) {
		logStream() << message << "\n";
	}
}

// -----
// main
// -----
int main(const int argc, const char * argv[]) {
	if (argc <= 1) // Just the program name, apparently.
			{
		printUsage(argv[0], logStream());
		return EXIT_FAILURE;
	}
	const std::string scriptFile = argv[1];
	const CmdLineFlags flags = scanCmdLineFlags(argc, argv);
	if (flags.verbose) {
		flags.print(logStream());
	}
	LSCompiler compiler; // Script parsing and bytecode output.
	LSVM vm;             // Executes bytecode produced by a Compiler.
	try {
		compiler.debugMode = flags.debug;
		compiler.enableWarnings = flags.warnings;
		// Parse & compile:
		if (flags.parse) {
			vPrint(flags,
					"LavaScript: Parsing script \"" + scriptFile + "\"...");
			compiler.parseScript(&vm, scriptFile);
		}
		if (flags.compile) {
			vPrint(flags, "LavaScript: Compiling...");
			compiler.compile(&vm);
		}
		// Debug printing:
		if (flags.dumpSyntax) {
			logStream() << compiler.syntTree << "\n";
		}
		if (flags.dumpSymbols) {
			logStream() << compiler.symTable << "\n";
		}
		if (flags.dumpTypes) {
			logStream() << vm.types << "\n";
		}
		if (flags.dumpFuncs) {
			logStream() << vm.functions << "\n";
		}
		if (flags.dumpIntermediateCode) {
			logStream() << compiler << "\n";
		}
		if (flags.dumpVMBytecode) {
			logStream() << vm.code << "\n";
		}
		// Exec:
		if (flags.run) {
			vPrint(flags, "LavaScript: Running script program...");
			vm.execute();
		}
		// Post execution memory dumps:
		if (flags.dumpGlobalData) {
			logStream() << vm.data << "\n";
		}
		if (flags.dumpGCObjs) {
			logStream() << vm.gc << "\n";
		}
		vPrint(flags, "LavaScript: Finished running script program.");
	} catch (...) {
#if LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
		if (!vm.callstack.isEmpty()) {
			vm.printStackTrace(logStream());
		}
#endif // LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
		logStream() << color::red() << "terminating with error(s)...\n"
				<< color::restore();
		return EXIT_FAILURE;
	}
}
