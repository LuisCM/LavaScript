// -----------------
// Integer numbers:
// -----------------
let i_val_1       =  42; // type=long
let i_val_2: int  = -1;
let i_val_3: long = +123;
let i_val_4: int  =  0xAAAAAAAA;
let i_val_5: long =  0x0BBBBBBBCCCCCCCC;
let i_val_6: int; // implicitly zero

assert(i_val_1 ==  42);
assert(i_val_2 == -1);
assert(i_val_3 == +123);
assert(i_val_4 ==  0xAAAAAAAA);
assert(i_val_5 ==  0x0BBBBBBBCCCCCCCC);
assert(i_val_6 ==  0);

// Due to the way runtime Variants are implemented, the underlaying
// type of individual variables is the largest common type available.
// So even if you specify 'int' explicitly, the var is stored as a long
// internally. The distinction only really becomes relevant when using
// typed arrays, in which case the data is then stored using the actual
// specified type to avoid wasting unnecessary storage space.
assert(typeOf(i_val_1) == typeOf(long));
assert(typeOf(i_val_2) == typeOf(long));
assert(typeOf(i_val_3) == typeOf(long));
assert(typeOf(i_val_4) == typeOf(long));
assert(typeOf(i_val_5) == typeOf(long));
assert(typeOf(i_val_6) == typeOf(long));

// ------------------------
// Floating-point numbers:
// ------------------------
let f_val_1         =  4.2; // type=double
let f_val_2: double = -0.5;
let f_val_3: double = +0.5;
let f_val_4: float  =  3.14;
let f_val_5: float; // implicitly zero

assert(f_val_1 ==  4.2);
assert(f_val_2 == -0.5);
assert(f_val_3 == +0.5);
assert(f_val_4 ==  3.14);
assert(f_val_5 ==  0.0);

assert(typeOf(f_val_1) == typeOf(double));
assert(typeOf(f_val_2) == typeOf(double));
assert(typeOf(f_val_3) == typeOf(double));
assert(typeOf(f_val_4) == typeOf(double));
assert(typeOf(f_val_5) == typeOf(double));

// ----------
// Booleans:
// ----------
let b_val_1 = true;
let b_val_2 = false;
let b_val_3: bool; // implicitly false/zero

assert(b_val_1 == true);
assert(b_val_2 == false);
assert(b_val_3 == false);

// Booleans also expand to integers (long, more specifically).
assert(typeOf(b_val_1) == typeOf(long));
assert(typeOf(b_val_2) == typeOf(long));
assert(typeOf(b_val_3) == typeOf(long));

// ---------
// Strings:
// ---------
let s_val_1      = "hello";
let s_val_2: str = "world";
let s_val_3      = s_val_1 + " " + s_val_2;
let s_val_4: str; // implicitly null

assert(s_val_1 == "hello");
assert(s_val_2 == "world");
assert(s_val_3 == "hello world");
assert(s_val_4 == null);

assert(typeOf(s_val_1) == typeOf(str));
assert(typeOf(s_val_2) == typeOf(str));
assert(typeOf(s_val_3) == typeOf(str));
assert(typeOf(s_val_4) == typeOf(str));

// --------
// Arrays:
// --------
// Individual arrays are homogeneous (same type for every item).
let arr_val_1        = [0, 1, 2, 3];
let arr_val_2: array = ["A", "B", "C"];
let arr_val_3: array; // implicitly null

// Note: element-wise comparison operators are not available for arrays
assert(arr_val_1[0] == 0);
assert(arr_val_1[1] == 1);
assert(arr_val_1[2] == 2);
assert(arr_val_1[3] == 3);

assert(arr_val_2[0] == "A");
assert(arr_val_2[1] == "B");
assert(arr_val_2[2] == "C");
assert(arr_val_3 == null);

assert(typeOf(arr_val_1) == typeOf(array));
assert(typeOf(arr_val_2) == typeOf(array));
assert(typeOf(arr_val_3) == typeOf(array));

assert(typeOf(arr_val_1[0]) == typeOf(long));
assert(typeOf(arr_val_2[0]) == typeOf(str));

// A multi-dimensional array can store sub-arrays of any type:
let the_matrix = [ ["A", "B", "C"], [1.1, 1.2, 1.3] ];
assert(typeOf(the_matrix)    == typeOf(array));
assert(typeOf(the_matrix[0]) == typeOf(array));
assert(typeOf(the_matrix[1]) == typeOf(array));

assert(the_matrix[0][0] == "A");
assert(the_matrix[0][1] == "B");
assert(the_matrix[0][2] == "C");
assert(the_matrix[1][0] == 1.1);
assert(the_matrix[1][1] == 1.2);
assert(the_matrix[1][2] == 1.3);

// ---------
// Ranges:
// ---------
let r_val_1        =  0..10;
let r_val_2: range = -5..5;
let r_val_3: range; // empty range; not null

assert(r_val_1 ==  0..10);
assert(r_val_2 == -5..5);
assert(r_val_3 ==  0..0);

assert(typeOf(r_val_1) == typeOf(range));
assert(typeOf(r_val_2) == typeOf(range));
assert(typeOf(r_val_3) == typeOf(range));

// ------------------------
// Function/callable type:
// ------------------------
let func_ref_1 = print;
let func_ref_2 = println;
let func_ref_3: function; // implicitly null

assert(typeOf(func_ref_1) == typeOf(function));
assert(typeOf(func_ref_2) == typeOf(function));
assert(typeOf(func_ref_3) == typeOf(function));

assert(func_ref_1 == print);
assert(func_ref_2 == println);
assert(func_ref_3 == null);

// -----
// Any:
// -----
let temp_lng: long   =  10;
let temp_dbl: double = -2.5;
let temp_str: str    =  "test";
let temp_arr: array  =  [1, 2, 3];
let temp_rng: range  =  0..9;

// typeOf() on any returns the underlaying type.
let any_val: any;
assert(any_val == null);
assert(typeOf(any_val) == typeOf(null));

any_val = temp_lng;
assert(any_val == 10);
assert(typeOf(any_val) == typeOf(long));

any_val = temp_dbl;
assert(any_val == -2.5);
assert(typeOf(any_val) == typeOf(double));

any_val = temp_str;
assert(any_val == "test");
assert(typeOf(any_val) == typeOf(str));

any_val = temp_arr;
assert(any_val == temp_arr);
assert(typeOf(any_val) == typeOf(array));

any_val = temp_rng;
assert(any_val == temp_rng);
assert(typeOf(any_val) == typeOf(range));

// ---------------
// Type Id (tid):
// ---------------
let t_int    = typeOf(int);
let t_long   = typeOf(long);
let t_float  = typeOf(float);
let t_double = typeOf(double);
let t_null: tid; // uninitialized tid == null

// the type of the var is 'tid'
assert(typeOf(t_int)    == typeOf(tid));
assert(typeOf(t_long)   == typeOf(tid));
assert(typeOf(t_float)  == typeOf(tid));
assert(typeOf(t_double) == typeOf(tid));
assert(typeOf(t_null)   == typeOf(tid));

// but it holds a type id value
assert(t_int    == typeOf(int));
assert(t_long   == typeOf(long));
assert(t_float  == typeOf(float));
assert(t_double == typeOf(double));
assert(t_null   == null); // Not the same as typeOf(null)!!!

// -----------
// Null type:
// -----------
let null_val = null;
assert(null_val == null);
assert(typeOf(null_val) == typeOf(null));

// LS_SRC_FILE_NAME, LS_SRC_LINE_NUM and LS_SRC_FUNC_NAME
// are built-in predefined globals with the corresponding
// source filename, current line number and current function name.
println("Finished running test script ", LS_SRC_FILE_NAME);
