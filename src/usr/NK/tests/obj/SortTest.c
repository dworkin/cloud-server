#include <NKlib.h>

inherit Test;

private Sort sort;

sortShouldSortArrayOfIntegers(void) {
    int *ints;

    ints = sort->sort(({ 3, 1, 4, -2, 0, 5 }));

    expectEqual(TEST_LINE, 6, sizeof(ints));
    expectEqual(TEST_LINE, -2, ints[0]);
    expectEqual(TEST_LINE, 0, ints[1]);
    expectEqual(TEST_LINE, 1, ints[2]);
    expectEqual(TEST_LINE, 3, ints[3]);
    expectEqual(TEST_LINE, 4, ints[4]);
    expectEqual(TEST_LINE, 5, ints[5]);
}

sortShouldSortArrayOfFloats(void) {
    float *floats;

    floats = sort->sort(({ 3., 1., 4., -2., 0., 5. }));

    expectEqual(TEST_LINE, 6, sizeof(floats));
    expectEqual(TEST_LINE, -2., floats[0]);
    expectEqual(TEST_LINE, 0., floats[1]);
    expectEqual(TEST_LINE, 1., floats[2]);
    expectEqual(TEST_LINE, 3., floats[3]);
    expectEqual(TEST_LINE, 4., floats[4]);
    expectEqual(TEST_LINE, 5., floats[5]);
}

sortShouldSortArrayOfStrings(void) {
    string *strings;

    strings = sort->sort(({ "Abc", "aBc", "abC", "abc", "ABC", "3", "10", "2", "1" }));

    expectEqual(TEST_LINE, 9, sizeof(strings));
    expectEqual(TEST_LINE, "1", strings[0]);
    expectEqual(TEST_LINE, "10", strings[1]);
    expectEqual(TEST_LINE, "2", strings[2]);
    expectEqual(TEST_LINE, "3", strings[3]);
    expectEqual(TEST_LINE, "ABC", strings[4]);
    expectEqual(TEST_LINE, "Abc", strings[5]);
    expectEqual(TEST_LINE, "aBc", strings[6]);
    expectEqual(TEST_LINE, "abC", strings[7]);
    expectEqual(TEST_LINE, "abc", strings[8]);
}

sortShouldSortArrayOfLibStrings(void) {
    String *strings;

    strings = sort->sort(({
        new String("Abc"),
        new String("aBc"),
        new String("abC"),
        new String("abc"),
        new String("ABC"),
        new String("3"),
        new String("10"),
        new String("2"),
        new String("1")
    }));

    expectEqual(TEST_LINE, 9, sizeof(strings));
    expectEqual(TEST_LINE, "1", strings[0]->buffer()->chunk());
    expectEqual(TEST_LINE, "10", strings[1]->buffer()->chunk());
    expectEqual(TEST_LINE, "2", strings[2]->buffer()->chunk());
    expectEqual(TEST_LINE, "3", strings[3]->buffer()->chunk());
    expectEqual(TEST_LINE, "ABC", strings[4]->buffer()->chunk());
    expectEqual(TEST_LINE, "Abc", strings[5]->buffer()->chunk());
    expectEqual(TEST_LINE, "aBc", strings[6]->buffer()->chunk());
    expectEqual(TEST_LINE, "abC", strings[7]->buffer()->chunk());
    expectEqual(TEST_LINE, "abc", strings[8]->buffer()->chunk());
}

void runBeforeTests(void) {
    sort = new Sort();
}

void runTests(void) {
    sortShouldSortArrayOfIntegers();
    sortShouldSortArrayOfFloats();
    sortShouldSortArrayOfStrings();
    sortShouldSortArrayOfLibStrings();
}