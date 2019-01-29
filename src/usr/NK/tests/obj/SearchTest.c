#include <NKlib.h>

inherit Test;

private Search search;

private void searchForRational(void) {
    search = new Search(({
        new Rational(1, 10),
                new Rational(1, 4),
                new Rational(1, 3),
                new Rational(1, 2),
                new Rational(2, 3),
                new Rational(3, 4),
                new Rational(9, 10)
    }));

    expectEqual(TEST_LINE, "({ 2, 3 })", dump_value(search->binary(new Rational(1, 3)), ([])));
    expectEqual(TEST_LINE, "({ nil, 3 })", dump_value(search->binary(new Rational(3, 5)), ([])));
    expectEqual(TEST_LINE, "({ nil, 3 })", dump_value(search->binary(3), ([])));
    expectEqual(TEST_LINE, "({ nil, 3 })", dump_value(search->binary(0.3), ([])));
    expectEqual(TEST_LINE, "({ nil, 3 })", dump_value(search->binary(3.), ([])));
}

private void searchForInt(void) {
    search = new Search(({ 1, 2, 4, 6, 8, 10 }));

    expectEqual(TEST_LINE, "({ nil, 3 })", dump_value(search->binary(3), ([])));
    expectEqual(TEST_LINE, "({ 2, 1 })", dump_value(search->binary(4), ([])));
    expectEqual(TEST_LINE, "({ 3, 3 })", dump_value(search->binary(6), ([])));
}

private void searchForFloat(void) {
    search = new Search(({ 1.1, 2.2, 4.3, 6.4, 8.5, 10.6 }));

    expectEqual(TEST_LINE, "({ nil, 3 })", dump_value(search->binary(8.4), ([])));
    expectEqual(TEST_LINE, "({ 4, 2 })", dump_value(search->binary(8.5), ([])));
    expectEqual(TEST_LINE, "({ 2, 1 })", dump_value(search->binary(4.3), ([])));
    expectEqual(TEST_LINE, "({ nil, 3 })", dump_value(search->binary(4.33), ([])));
}

void runTests(void) {
    searchForRational();
    searchForInt();
    searchForFloat();
}