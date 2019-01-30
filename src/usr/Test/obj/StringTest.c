#include <Test.h>

inherit Test;

private Stringify stringify;
private LongestString longestString;
private TrimString trimString;
private BoxifyString boxifyString;

private void longestStringShouldFindLongestString(void) {
    string *x;

    x = ({ "1", "222", "33" });

    expectEqual(TEST_LINE, 3, longestString->evaluate(x));
}

private void stringifyShouldStringifyArray(void) {
    string *x;

    x = ({ "1", "222", "33" });

    expectEqual(TEST_LINE, "({ \"1\", \"222\", \"33\" })", stringify->evaluate(x));
}

private void stringifyShouldStringifyMapping(void) {
    mapping x;

    x = ([ "one": "two", "three": "four" ]);

    expectEqual(TEST_LINE, "([ \"one\":\"two\", \"three\":\"four\" ])", stringify->evaluate(x));
}

private void stringifyShouldStringifyObjectWithToString(void) {
    Rational x;

    x = new Rational(0.83333333);

    expectEqual(TEST_LINE, "5/6", stringify->evaluate(x));
}

private void stringifyShouldStringifyObject(void) {
    Random x;

    x = new Random();

    expectEqual(TEST_LINE, "</@@@/statistics/Random#-1>", stringify->evaluate(x));
}

private void stringifyShouldStringifyInteger(void) {
    int x;

    x = 42;
    expectEqual(TEST_LINE, "42", stringify->evaluate(x));

    x = -2147483647;
    expectEqual(TEST_LINE, "-2147483647", stringify->evaluate(x));

    x = 2147483647;
    expectEqual(TEST_LINE, "2147483647", stringify->evaluate(x));
}

private void stringifyShouldStringifyFloat(void) {
    float x;

    x = 42.;
    expectEqual(TEST_LINE, "42", stringify->evaluate(x));

    x = 42.424242;
    expectEqual(TEST_LINE, "42.424242", stringify->evaluate(x));

    x = 0.4242424246;
    expectEqual(TEST_LINE, "0.424242425", stringify->evaluate(x));

    x = 2.22507386e-308;
    expectEqual(TEST_LINE, "2.22507386e-308", stringify->evaluate(x));

    x = 1.79769313e+308;
    expectEqual(TEST_LINE, "1.79769313e+308", stringify->evaluate(x));

    x = 1.79769313e308;
    expectEqual(TEST_LINE, "1.79769313e+308", stringify->evaluate(x));
}

private void stringifyShouldStringifyString(void) {
    string x;

    x = "42";

    expectEqual(TEST_LINE, "42", stringify->evaluate(x));
}

private void trimStringShouldTrimString(void) {
    string x;

    x = "     lorem\tipsum\bdolor\ramet     \n     lorem \t ipsum \b dolor \r amet    ";

    expectEqual(TEST_LINE, "lorem ipsum dolor amet lorem ipsum dolor amet",
            trimString->evaluate(x));
}

private void spacesShouldReturnedExpectedSpaces(void) {
    expectEqual(TEST_LINE, "", spaces(-1));
    expectEqual(TEST_LINE, "", spaces(0));
    expectEqual(TEST_LINE, " ", spaces(1));
    expectEqual(TEST_LINE, "  ", spaces(2));
}

private void repeatShouldReturnExpectedString(void) {
    expectEqual(TEST_LINE, "", repeat("x", -1));
    expectEqual(TEST_LINE, "", repeat("x", 0));
    expectEqual(TEST_LINE, "x", repeat("x", 1));
    expectEqual(TEST_LINE, "xx", repeat("x", 2));
    expectEqual(TEST_LINE, "xxx", repeat("x", 3));
}

private void boxifyStringShouldRenderDefaultBox(void) {
    string str;

    str = "one\nlong str\ntwo";

    boxifyString = new BoxifyString();
    expectEqual(TEST_LINE, " +----------+\n" +
                           " | one      |\n" +
                           " | long str |\n" +
                           " | two      |\n" +
                           " +----------+", boxifyString->evaluate(str));

    boxifyString = new BoxifyString(BOX_STRING_SINGLE);
    expectEqual(TEST_LINE, " ┌──────────┐\n" +
                           " │ one      │\n" +
                           " │ long str │\n" +
                           " │ two      │\n" +
                           " └──────────┘", boxifyString->evaluate(str));

    boxifyString = new BoxifyString(BOX_STRING_DOUBLE);
    expectEqual(TEST_LINE, " ╔══════════╗\n" +
                           " ║ one      ║\n" +
                           " ║ long str ║\n" +
                           " ║ two      ║\n" +
                           " ╚══════════╝", boxifyString->evaluate(str));

    boxifyString = new BoxifyString(BOX_STRING_DOUBLE_HORIZONTAL);
    expectEqual(TEST_LINE, " ╔══════════╗\n" +
                           " │ one      │\n" +
                           " │ long str │\n" +
                           " │ two      │\n" +
                           " ╚══════════╝", boxifyString->evaluate(str));

    boxifyString = new BoxifyString(BOX_STRING_ROUND);
    expectEqual(TEST_LINE, " ╭──────────╮\n" +
                           " │ one      │\n" +
                           " │ long str │\n" +
                           " │ two      │\n" +
                           " ╰──────────╯", boxifyString->evaluate(str));
}

void runBeforeTests(void) {
    stringify = new Stringify();
    longestString = new LongestString();
    trimString = new TrimString();
}

void runTests(void) {
    longestStringShouldFindLongestString();
    stringifyShouldStringifyArray();
    stringifyShouldStringifyMapping();
    stringifyShouldStringifyObjectWithToString();
    stringifyShouldStringifyObject();
    stringifyShouldStringifyInteger();
    stringifyShouldStringifyFloat();
    stringifyShouldStringifyString();
    trimStringShouldTrimString();
    spacesShouldReturnedExpectedSpaces();
    repeatShouldReturnExpectedString();
    boxifyStringShouldRenderDefaultBox();
}