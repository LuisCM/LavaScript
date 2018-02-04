// -------------------------------------------------------
// Script functions that will be called from the C++ code
// -------------------------------------------------------

func scriptFunc0()
    println(LS_SRC_FUNC_NAME);
    println("No arguments\n");
end

func scriptFunc1(arg0: int) -> int
    println(LS_SRC_FUNC_NAME);
    println("arg0 = ", arg0, "\n");
    return 321;
end

func scriptFunc2(arg0: int, arg1: float) -> float
    println(LS_SRC_FUNC_NAME);
    println("arg0 = ", arg0);
    println("arg1 = ", arg1, "\n");
    return -1.5;
end

func scriptFunc3(arg0: int, arg1: float, arg2: str) -> str
    println(LS_SRC_FUNC_NAME);
    println("arg0 = ", arg0);
    println("arg1 = ", arg1);
    println("arg2 = ", arg2, "\n");
    return "testing, 123";
end

func scriptFuncVarargs(args...)
    println(LS_SRC_FUNC_NAME);
    for arg in args do
        println("var arg: ", arg);
    end
    println();
end

println("Finished running test script ", LS_SRC_FILE_NAME);
