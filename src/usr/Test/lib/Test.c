#include <Test.h>

inherit Terminal;
inherit NK_KFUN;

private string path;
private object collector;
private object reportTo;
private int tests;
private int pass;
private int fail;
private mapping testResults;

private reportResults(void) {
    collector->resultsFrom(path, testResults);
}

nomask static void report(string str) {
    reportTo->message(str);
}

private void handleFail(string str) {
    testResults["fail"][fail++] = red(eks()) + " " + str;
}

private void handlePass(string str) {
    testResults["pass"][pass++] = green(check()) + " " + str;
}

nomask static string testLine(mixed **trace) {
    mixed *x;
    int sz;

    sz = sizeof(trace);
    x = trace[sz - 2];

    return x[TRACE_FUNCTION] + ":" + x[TRACE_LINE];
}

nomask static void expectTrue(string str, mixed actual) {
    tests++;

    if (!actual) {
        handleFail(str);
    } else {
        handlePass(str);
    }
}

nomask static void expectFalse(string str, mixed actual) {
    expectTrue(str, !actual);
}

nomask static void expectEqual(string str, mixed expected, mixed actual) {
    int x;

    if (typeof(expected) == T_FLOAT && typeof(actual) == T_FLOAT) {
        if (expected == actual) {
            x = TRUE;
        } else {
            x = fabs(actual - expected) < TOLERANCE;
        }
    } else {
        x = actual == expected;
    }

    expectTrue("[" + str + "] [" + expected + "] [" + actual + "]", x);
}

static void runBeforeTests(void) {

}

static void runTests(void) {

}

nomask void _runTests(object reportTo, object collector) {
    string *str;

    str = explode(object_name(this_object()), "#");
    path = str[0] + ".c";

    ::reportTo = reportTo;
    ::collector = collector;
    tests = 0;
    pass = 0;
    fail = 0;
    testResults = ([ ]);
    testResults["pass"] = allocate(1000);
    testResults["fail"] = allocate(1000);

    report("\nRunning tests in " + object_name(this_object()) + "\n");
    runBeforeTests();
    runTests();
    report("\tTests: " + tests + ", pass: " + pass + ", fail: " + fail + "\n");
    reportResults();
    destruct_object(this_object());
}
