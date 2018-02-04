// -------------------------
// Test Fibonacci Recursive
// -------------------------

func fibonacciRecursive(n: int) -> int
    if n <= 1 then
        return n;
    end
    return fibonacciRecursive(n - 1) + fibonacciRecursive(n - 2);
end

let fib1 = fibonacciRecursive(15);
println("Fibonacci recursive of 15 = ", fib1);
assert(fib1 == 610);

// -------------------------
// Test Fibonacci Iterative
// -------------------------

func fibonacciIterative(n: int) -> int
    let a: int = 0;
    let b: int = 1;
    let c: int = 0;

    if n == 0 then
        return a;
    end

    let count = n + 1;
    for i in 2..count do
        c = a + b;
        a = b;
        b = c;
    end
    return b;
end

let fib2 = fibonacciIterative(15);
println("Fibonacci iterative of 15 = ", fib2);
assert(fib2 == 610);

println("Finished running test script ", LS_SRC_FILE_NAME);
