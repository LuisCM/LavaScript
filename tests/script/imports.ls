// ---------------------------------------
// Test script that imports other scripts
// ---------------------------------------

import "tests/script/test-import-1.ls";
import "tests/script/test-import-2.ls";

// Functions declared in the imported scripts:
import1_func();
import2_func();

func main()
    // Variables declared in the imported scripts:
    println("import1_var = ", import1_var);
    println("import2_var = ", import2_var);
    println("Finished running test script ", LS_SRC_FILE_NAME);
end

main();
