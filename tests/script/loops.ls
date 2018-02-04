// ----------------------------------
// A few basic tests for script loops
// ----------------------------------

// -----------------
// Test loop simple
// -----------------

func testLoopSimple()
    let i = 0;
    loop
        if i == 10 then
            break;
        end
        println("looping; i = ", i);
        i += 1;
    end
    assert(i == 10);
    println(LS_SRC_FUNC_NAME, "() - OK\n");
end

testLoopSimple();

// ----------------
// Test while loop
// ----------------

func testWhileLoop()
    let i: int;

    // Loop with condition:
    i = 0;
    while i < 10 do
        println("looping; i = ", i);
        i += 1;
    end
    assert(i == 10);

    // Infinite loop with break condition:
    i = 0;
    while true do
        if i == 5 then
            break;
        end
        println("looping; i = ", i);
        i += 1;
    end
    assert(i == 5);

    // Test the 'continue' jump:
    let times_printed = 0;
    i = 0;
    while i != 10 do
        i += 1;
        if (i % 2) == 0 then
            println("looping; i = ", i);
            times_printed += 1;
            continue;
        end
    end
    assert(times_printed == 5);
    println(LS_SRC_FUNC_NAME, "() - OK\n");
end

testWhileLoop();

// --------------
// Test for loop
// --------------

func testForLoop()
    // Range iteration:
    let times_printed = 0;

    // For loop on range literal:
    for i in 0..10 do
        println("looping; i = ", i);
        times_printed += 1;
    end
    assert(times_printed == 10);

    // For loop on range variable:
    times_printed = 0;
    let r = 0..5;
    for i in r do
        println("looping; i = ", i);
        times_printed += 1;
    end
    assert(times_printed == 5);

    // Array iteration:
    let a = ["A", "B", "C", "D"];
    let n = 0;

    // Array literal:
    for e in ["A", "B", "C", "D"] do
        println("Accessing array element: ", e);
        assert(a[n] == e);
        n += 1;
    end

    // For loop on an Array variable:
    n = 0;
    for e in a do
        println("Accessing array element: ", e);
        assert(a[n] == e);
        n += 1;
    end

    // For loop on array of arrays (contents of the array must be another array or range):
    let array2d = [ [1, 2, 3], [4, 5, 6] ];
    for e in array2d[1] do
        print("Accessing array element: ", e, " ");
        println(e as long); // 'e' is any, so the cast is need to reach the value.
    end

    println(LS_SRC_FUNC_NAME, "() - OK\n");
end

testForLoop();

// ------------------------
// Test for loop arguments 
// ------------------------

func testForLoopOnArguments(arg0: array, arg1: range)
    // Argument is an array:
    for i in arg0 do
        print("looping: ");
        println(i); // Type = any
    end

    // Argument is a range:
    for i in arg1 do
        print("looping: ");
        println(i); // Type = long
    end

    println(LS_SRC_FUNC_NAME, "() - OK\n");
end

testForLoopOnArguments(["A", "B", "C", "D"], -2..2);

// -------------
// Test args...
// -------------

func varArgsFunc(args...)
    for arg in args do
        println("var arg: ", arg);
    end
    println(LS_SRC_FUNC_NAME, "() - OK\n");
end

varArgsFunc(42, 6.66, "hello world", 1..5, [11, 22, 33]);

// ------------------------------
// Test function array and range 
// ------------------------------

func getArray() -> array return ["one", "two", "three", "four"]; end
func getRange() -> range return 2..8; end

//
// Function call returning array or range:
// (also making sure for-loop works are the global scope)
//
for i in getArray() do
    print("looping: ");
    println(i as str);
end
for i in getRange() do
    print("looping: ");
    println(i);
end

println("Global for-loop with function arg - OK\n");

println("Finished running test script ", LS_SRC_FILE_NAME);
