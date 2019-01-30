#include <Test.h>

inherit NK_KFUN;

private object reportTo;
private mapping allTestResults;
private int startTimeT;
private float startTimeM;
private int stopTimeT;
private float stopTimeM;
private int suites;
private int suitesExpected;
private int testCount;
private int failCount;
private int passCount;

void resultsFrom(string path, mapping testResults) {
    allTestResults[path] = (testResults["fail"] - ({ nil }));
    suites--;
    failCount += sizeof(testResults["fail"] - ({ nil }));
    passCount += sizeof(testResults["pass"] - ({ nil }));
}

static void processFails(void) {
    Array array;
    Function reducer;
    Iterator iterator;
    string *indices;
    string index;

    indices = map_indices(allTestResults);
    reducer = new ArrayToListReducer();
    iterator = new IntIterator(0, sizeof(indices) - 1);
    reportTo->message("\n");

    if (suites) {
        reportTo->message("Failed test suites: " + suites + " / " + suitesExpected + "\n");
    }

    while (!iterator->end()) {
        index = indices[iterator->next()];
        array = new Array(allTestResults[index]);
        if (array->size()) {
            reportTo->message(index + "\n");
            reportTo->message(array->reduce(reducer, 0, 0, 1) + "\n");
        }
    }
}

static void reportDone(void) {
    float f;

    ({ stopTimeT, stopTimeM }) = millitime();
    f = ((float) stopTimeT + stopTimeM) - ((float) startTimeT + startTimeM);

    if (!suites) {
        testCount = passCount + failCount;
        reportTo->message("Tests: " + testCount + ", pass: " + passCount + ", fail: " + failCount + ".\n");
        reportTo->message("Tests run in " + f + " s.\n");
    }

    reportTo->message("\nDone.\n");
    reportTo->showPrompt();
}

static void afterTests(string *str) {
    Iterator iterator;

    iterator = new IntIterator(0, sizeof(str) - 1);
    while (!iterator->end()) {
        iterator->next();
        while (find_object(str[iterator->current()])) {
            destruct_object(str[iterator->current()]);
        }
    }
}

void runTests(object reportTo, varargs string specOverride) {
    object *tests;
    object test;
    string *fnames, *objects;
    string path, str;
    Iterator iterator;
    Continuation continuation;

    ({ startTimeT, startTimeM }) = millitime();

    allTestResults = ([ ]);
    ::reportTo = reportTo;
    path = TEST_OBJ_PATH;
    fnames = get_dir(path + (specOverride ? specOverride : TEST_SPEC_PATH))[0];
    iterator = new IntIterator(0, sizeof(fnames) - 1);
    tests = allocate(sizeof(fnames));
    objects = allocate(sizeof(fnames));

    while (!iterator->end()) {
        str = path + "/" + fnames[iterator->next()];
        str = str[0 .. strlen(str) - 3];

        while (find_object(str)) {
            destruct_object(str);
        }

        compile_object(str);
        tests[iterator->current()] = clone_object(str);
        objects[iterator->current()] = str;
    }

    suites = sizeof(objects);
    suitesExpected = suites;
    testCount = 0;
    passCount = 0;
    failCount = 0;

    continuation = new DistContinuation(tests, 1, "_runTests", reportTo, this_object());
    continuation += new Continuation("processFails");
    continuation += new Continuation("afterTests", objects);
    continuation += new Continuation("reportDone");
    continuation->runNext();
}