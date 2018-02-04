// -----------------------------------------------------
// Global variables that will be edited by the C++ code
// -----------------------------------------------------

let a_integer = 42;
let a_float    = 2.5;
let a_range    = 0..9;
let a_string   = "hello from script";
let a_tid      = typeOf(int);

func printGlobals()
    println("int    = ", a_integer);
    println("float  = ", a_float);
    println("range  = ", a_range);
    println("string = ", a_string);
	println("typeOf = ", a_tid);
end

printGlobals();

println("Finished running test script ", LS_SRC_FILE_NAME);
