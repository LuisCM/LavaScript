// ------------------------------------------------
// A few basic tests for script-declared functions
// ------------------------------------------------

// ------
// void
// ------

func voidFunction()
    println("I return nothing");
end

// ------
// bool
// ------

func getBool() -> bool
    return true;
end

assert(getBool() == true);

// -----
// int
// -----

func getInt() -> int
    return 42;
end

assert(getInt() == 42);

// ------
// float
// ------

func getFloat() -> float
    return 4.2;
end

assert(getFloat() == 4.2);

// ----
// str
// ----

func getStr() -> str
    return "hello";
end

assert(getStr() == "hello");

// -------
// typeOf
// -------

func getTypeId() -> tid
    return typeOf(true);
end

assert(getTypeId() == typeOf(true));

// ------
// range
// ------

func getRange() -> range
    return -2..2;
end

assert(getRange() == -2..2);

// ------
// array
// ------

func getArray() -> array
    return ["A", "B", "C", "D"];
end

assert(getArray()[0] == "A");
assert(getArray()[1] == "B");
assert(getArray()[2] == "C");
assert(getArray()[3] == "D");

// ------------
// array index
// ------------

func getArrayIndex(idx: int) -> str
    let arr = ["A", "B", "C", "D"];
    return arr[idx];
end

assert(getArrayIndex(0) == "A");
assert(getArrayIndex(1) == "B");
assert(getArrayIndex(2) == "C");
assert(getArrayIndex(3) == "D");

// -----
// long
// -----

let gNumber: long = 666;

func getGlobal() -> long
    return gNumber;
end

assert(getGlobal() == gNumber);
assert(getGlobal() == 666);

// -------
// object
// -------

func getNullObj() -> object
    return null;
end

assert(getNullObj() == null);

// ---------
// function
// ---------

func getFunction() -> function
    return getStr;
end

let funcRef = getFunction();

assert(getFunction() == getStr);
assert(funcRef() == getStr());

// ----
// any
// ----

func getAny() -> any
    let strVal = "hello world";
    return strVal;
end

assert(typeOf(getAny()) == typeOf(str));
assert(getAny() == "hello world");

// -------
// struct
// -------

type MyObject struct
    dummy: int,
end

func getMyObj() -> MyObject
    return MyObject{ 42 };
end

func getMyObjIndirect() -> MyObject
    return getMyObj();
end

assert(typeOf(getMyObj()) == typeOf(MyObject));
assert(typeOf(getMyObjIndirect()) == typeOf(MyObject));

// ---------
// function
// ---------

func myFunc1(arg: str)
    assert(arg == "indirect call test");
end

func myFunc2(arg: int)
    assert(arg == 666);
end

func callIndirect(fn1: function, arg1: str, arg2: int)
    let fn2 = myFunc2;
    fn1(arg1); // => can only be resolved at runtime
    fn2(arg2); // => statically resolved to myFunc2
end

callIndirect(myFunc1, "indirect call test", 666);

// -------
// params
// -------

func testFuncParams(p0: int, p1: float, p2: str, p3: bool, p4: array, p5: tid, p6: tid, p7: function)
    assert(p0 == 11);
    assert(p1 == 1.1);
    assert(p2 == "foobar");
    assert(p3 == true);
    assert(p4[0] == 4);
    assert(p4[1] == 5);
    assert(p5 == typeOf(array));
    assert(p6 == typeOf(function));
    assert(p7 == myFunc1);
end

testFuncParams(11, 1.1, "foobar", (1 < 2), [4, 5], typeOf(array), typeOf(myFunc1), myFunc1);

// -----
// args
// -----

func varArgsFunc(args...)
    assert(args[0] == 33);
    assert(args[1] == 44);
    assert(args[2] == 55);
    assert(args[3] == "hello world");
    assert(args[4] == 3.14);
    assert(args[5] == myFunc1);
end

varArgsFunc(33, 44, 55, "hello world", 3.14, myFunc1);

// ----------
// recursive
// ----------

// Recursive function with stop condition from the passed parameter.
func testRecursion(numCalls: int)
    if numCalls == 10 then
        return;
    end
    println("numCalls = ", numCalls);
    testRecursion(numCalls + 1);
end

testRecursion(0);

println("Finished running test script ", LS_SRC_FILE_NAME);
