#include <NKlib.h>

inherit Test;
inherit NK_KFUN;

private Function testFunction;
private Function toBoxReducer;
private Array array;
private Array c;
private Array d;
private Array one;
private Array empty;
private Array a0;
private Array a1;
private Array a2;
private Array a3;
private Array a4;
private Array a5;
private Array a6;
private Array a7;

private void toArrayShouldReturnUnderpinningArray(void) {
    mixed *actual;

    actual = array->toArray();

    expectEqual(TEST_LINE, 3, sizeof(actual));
    expectEqual(TEST_LINE, -1., actual[0]);
    expectEqual(TEST_LINE, 0., actual[1]);
    expectEqual(TEST_LINE, 1., actual[2]);
}

private void sizeShouldReturnSizeOfUnderpinningArray(void) {
    expectEqual("Wrong array size()", 3, array->size());
}

private void _testFilter(string str, int times, varargs int from, int to) {
    testFunction = new TestFunction();

    array->filter(testFunction, from, to);

    expectEqual(str, times, testFunction->getCalledTimes());
}

private void filterShouldFilterFromGivenParameters(void) {
    _testFilter("Wrong number of filter calls 1", 3 * 2);
    _testFilter("Wrong number of filter calls 2", 3 * 2, 0);
    _testFilter("Wrong number of filter calls 3", 3 * 2 - 2, 1);
    _testFilter("Wrong number of filter calls 4", 3 * 2 - 2, 1, 2);
}

private void _testMap(string str, int times, varargs int from, int to) {
    testFunction = new TestFunction();

    array->map(testFunction, from, to);

    expectEqual(str, times, testFunction->getCalledTimes());
}

private void mapShouldMapFromGivenParameters(void) {
    _testMap("Wrong number of map calls 1", 3);
    _testMap("Wrong number of map calls 2", 3, 0);
    _testMap("Wrong number of map calls 3", 2, 1);
    _testMap("Wrong number of map calls 4", 2, 1, 2);
}

private void _testReduce(string str, int times, varargs int from, int to, int evalFirst) {
    testFunction = new TestFunction();

    array->reduce(testFunction, from, to, evalFirst);

    expectEqual(str, times, testFunction->getCalledTimes());
}

private void reduceShouldReduceFromGivenParameters(void) {
    _testReduce("Wrong number of reduce calls 1", 2);
    _testReduce("Wrong number of reduce calls 2", 2, 0);
    _testReduce("Wrong number of reduce calls 3", 1, 1);
    _testReduce("Wrong number of reduce calls 4", 1, 1, 2);
    _testReduce("Wrong number of reduce calls 5", 3, 0, 0, TRUE);
    _testReduce("Wrong number of reduce calls 6", 2, 0, 0, FALSE);
}

private void arrayShouldBeIterable(void) {
    Iterator iterator;

    iterator = new Iterator(array, 0, 2);

    expectFalse(TEST_LINE, iterator->end());
    expectEqual(TEST_LINE, -1. , iterator->next());
    expectEqual(TEST_LINE, -1. , iterator->current());
    expectEqual(TEST_LINE, 0. , iterator->next());
    expectEqual(TEST_LINE, 0. , iterator->current());
    expectEqual(TEST_LINE, 1. , iterator->next());
    expectEqual(TEST_LINE, 1. , iterator->current());
    expectTrue(TEST_LINE, iterator->end());
}

private void arrayShouldBeIndexable(void) {
    Array actual;

    actual = array[0..];
    expectEqual(TEST_LINE, 3, actual->size());
    expectEqual(TEST_LINE, -1., actual[0]);

    actual = array[..2];
    expectEqual(TEST_LINE, 3, actual->size());
    expectEqual(TEST_LINE, -1., actual[0]);

    actual = array[0..1];
    expectEqual(TEST_LINE, 2, actual->size());
    expectEqual(TEST_LINE, -1., actual[0]);
    expectEqual(TEST_LINE, 0., actual[1]);

    actual = array[1..2];
    expectEqual(TEST_LINE, 2, actual->size());
    expectEqual(TEST_LINE, 0., actual[0]);
    expectEqual(TEST_LINE, 1., actual[1]);

    actual = array[..];
    expectEqual(TEST_LINE, 3, actual->size());
    expectEqual(TEST_LINE, -1., actual[0]);
}

private void arrayIndexAssignment(void) {
    array[1] = 42.;

    expectEqual(TEST_LINE, -1., array[0]);
    expectEqual(TEST_LINE, 42., array[1]);
    expectEqual(TEST_LINE, 1., array[2]);
}

private void unionShouldCreateUnionOfSets(void) {
    Array x;

    x = empty->union(one);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = empty | one;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->union(empty);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one | empty;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->union(one);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one | one;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = empty->union(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty | empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = c->union(d);

    expectEqual(TEST_LINE, 8, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 5., x->get(2));
    expectEqual(TEST_LINE, 6., x->get(3));
    expectEqual(TEST_LINE, 8., x->get(4));
    expectEqual(TEST_LINE, 1., x->get(5));
    expectEqual(TEST_LINE, 3., x->get(6));
    expectEqual(TEST_LINE, 7., x->get(7));

    x = c | d;

    expectEqual(TEST_LINE, 8, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 5., x->get(2));
    expectEqual(TEST_LINE, 6., x->get(3));
    expectEqual(TEST_LINE, 8., x->get(4));
    expectEqual(TEST_LINE, 1., x->get(5));
    expectEqual(TEST_LINE, 3., x->get(6));
    expectEqual(TEST_LINE, 7., x->get(7));

    x = d->union(c);

    expectEqual(TEST_LINE, 8, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 5., x->get(2));
    expectEqual(TEST_LINE, 7., x->get(3));
    expectEqual(TEST_LINE, 8., x->get(4));
    expectEqual(TEST_LINE, 2., x->get(5));
    expectEqual(TEST_LINE, 4., x->get(6));
    expectEqual(TEST_LINE, 6., x->get(7));

    x = d | c;

    expectEqual(TEST_LINE, 8, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 5., x->get(2));
    expectEqual(TEST_LINE, 7., x->get(3));
    expectEqual(TEST_LINE, 8., x->get(4));
    expectEqual(TEST_LINE, 2., x->get(5));
    expectEqual(TEST_LINE, 4., x->get(6));
    expectEqual(TEST_LINE, 6., x->get(7));
}

private void intersectionShouldCreateIntersectionOfSets(void) {
    Array x;

    x = empty->intersection(one);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty & one;

    expectEqual(TEST_LINE, 0, x->size());

    x = one->intersection(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = one & empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = one->intersection(one);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one & one;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = empty->intersection(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty & empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = c->intersection(d);

    expectEqual(TEST_LINE, 2, x->size());
    expectEqual(TEST_LINE, 5., x->get(0));
    expectEqual(TEST_LINE, 8., x->get(1));

    x = c & d;

    expectEqual(TEST_LINE, 2, x->size());
    expectEqual(TEST_LINE, 5., x->get(0));
    expectEqual(TEST_LINE, 8., x->get(1));

    x = d->intersection(c);

    expectEqual(TEST_LINE, 2, x->size());
    expectEqual(TEST_LINE, 5., x->get(0));
    expectEqual(TEST_LINE, 8., x->get(1));

    x = c & d;

    expectEqual(TEST_LINE, 2, x->size());
    expectEqual(TEST_LINE, 5., x->get(0));
    expectEqual(TEST_LINE, 8., x->get(1));
}

private void symmetricDifferenceShouldCreateSymDifOfSets(void) {
    Array x;

    x = empty->symmetricDifference(one);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = empty ^ one;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->symmetricDifference(empty);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one ^ empty;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->symmetricDifference(one);

    expectEqual(TEST_LINE, 0, x->size());

    x = one ^ one;

    expectEqual(TEST_LINE, 0, x->size());

    x = empty->symmetricDifference(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty ^ empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = c->symmetricDifference(d);

    expectEqual(TEST_LINE, 6, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 6., x->get(2));
    expectEqual(TEST_LINE, 1., x->get(3));
    expectEqual(TEST_LINE, 3., x->get(4));
    expectEqual(TEST_LINE, 7., x->get(5));

    x = c ^ d;

    expectEqual(TEST_LINE, 6, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 6., x->get(2));
    expectEqual(TEST_LINE, 1., x->get(3));
    expectEqual(TEST_LINE, 3., x->get(4));
    expectEqual(TEST_LINE, 7., x->get(5));

    x = d->symmetricDifference(c);

    expectEqual(TEST_LINE, 6, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 7., x->get(2));
    expectEqual(TEST_LINE, 2., x->get(3));
    expectEqual(TEST_LINE, 4., x->get(4));
    expectEqual(TEST_LINE, 6., x->get(5));

    x = d ^ c;

    expectEqual(TEST_LINE, 6, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 7., x->get(2));
    expectEqual(TEST_LINE, 2., x->get(3));
    expectEqual(TEST_LINE, 4., x->get(4));
    expectEqual(TEST_LINE, 6., x->get(5));
}

private void differenceShouldCreateDifferenceOfSets(void) {
    Array x;

    x = empty->difference(one);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty - one;

    expectEqual(TEST_LINE, 0, x->size());

    x = one->difference(empty);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one - empty;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->difference(one);

    expectEqual(TEST_LINE, 0, x->size());

    x = one - one;

    expectEqual(TEST_LINE, 0, x->size());

    x = empty->difference(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty - empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = c->difference(d);

    expectEqual(TEST_LINE, 3, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 6., x->get(2));

    x = c - d;

    expectEqual(TEST_LINE, 3, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 6., x->get(2));

    x = d->difference(c);

    expectEqual(TEST_LINE, 3, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 7., x->get(2));

    x = d - c;

    expectEqual(TEST_LINE, 3, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 7., x->get(2));
}

private void getOutOfRangeShouldThrowExpectedError(void) {
    expectEqual(TEST_LINE, "Array: index -1 is out of range.", catch(array->get(-1)));
    expectEqual(TEST_LINE, "Array: index 1000 is out of range.", catch(array->get(1000)));
}

private void toMarkedListReducerCreateUnorderedList(void) {
    Function reducer;
    Rational r;
    Array x;

    r = new Rational(0.5);
    x = new Array(({ 1., r, 2. }));

    reducer = new ArrayToMarkedListReducer();

    expectEqual(TEST_LINE, "› 2\n" +
                                        "› 4\n" +
                                        "› 5\n" +
                                        "› 6\n" +
                                        "› 8\n", c->reduce(reducer, 0, c->size() - 1, 1));
    expectEqual(TEST_LINE, "› 1\n" +
                                        "› 1/2\n" +
                                        "› 2\n", x->reduce(reducer, 0, x->size() - 1, 1));
}

private void toMarkedListReducerCreateOrderedList(void) {
    Function reducer;
    Rational r;
    Array x;

    r = new Rational(0.5);
    x = new Array(({ 1., r, 2. }));

    reducer = new ArrayToMarkedListReducer(2);

    expectEqual(TEST_LINE, "2) 2\n" +
                                      "3) 4\n" +
                                      "4) 5\n" +
                                      "5) 6\n" +
                                      "6) 8\n", c->reduce(reducer, 0, c->size() - 1, 1));

    reducer->apply(2);
    expectEqual(TEST_LINE, "2) 1\n" +
                                "3) 1/2\n" +
                                "4) 2\n", x->reduce(reducer, 0, x->size() - 1, 1));
}

private void containsShouldIndicateIfArrayContainsValue(void) {
    Rational r;
    Array x;

    r = new Rational(0.5);
    x = new Array(({ 1., r, 2. }));

    expectTrue(TEST_LINE, c->contains(6.));
    expectFalse(TEST_LINE, c->contains(42.));
    expectTrue(TEST_LINE, x->contains(r));
    expectTrue(TEST_LINE, x->contains(new Rational(0.5)));
    expectFalse(TEST_LINE, x->contains(new Rational(0.55)));
    expectFalse(TEST_LINE, x->contains(new Random()));
}

private void tabulateShouldCreateTabulatedOutput(void) {
    mixed *items;
    string *actual;
    Array x;

    items = ({
        "one", "two", "three", "four", "five", "six",
        "onetwothree", "fourfivesix", "seven", "eight",
        "nine", "seveneightnight", "ten", "eleven",
        "twelve", "thirteen", "teneleventwelve"
    });
    x = new Array(items);

    actual = explode(x->tabulate(), "\n");

    expectEqual(TEST_LINE, 5, sizeof(actual));
    expectEqual(TEST_LINE,
            "one              two              three            four             ",
            actual[0]);
    expectEqual(TEST_LINE,
            "five             six              onetwothree      fourfivesix      ",
            actual[1]);
    expectEqual(TEST_LINE,
            "seven            eight            nine             seveneightnight  ",
            actual[2]);
    expectEqual(TEST_LINE,
            "ten              eleven           twelve           thirteen         ",
            actual[3]);
    expectEqual(TEST_LINE,
            "teneleventwelve  ",
            actual[4]);
}

private void uniqueShouldCreateUniqueArray(void) {
    Array b0, b1, b2, b3, b4, b5, b6, b7;

    b0 = a0->unique();
    b1 = a1->unique();
    b2 = a2->unique();
    b3 = a3->unique();
    b4 = a4->unique();
    b5 = a5->unique();
    b6 = a6->unique();
    b7 = a7->unique();

    expectEqual(TEST_LINE, 0, b0->size());
    expectEqual(TEST_LINE, "({ 1, 2 })", dump_value(b2->toArray(), ([])));
    expectEqual(TEST_LINE, "({ 1 })", dump_value(b3->toArray(), ([])));
    expectEqual(TEST_LINE, "({ 1, 2, 3, 4, 5, 6, 7 })", dump_value(b4->toArray(), ([])));
    expectEqual(TEST_LINE, "({ 1, 2, 3, 4, 5, 6, 7 })", dump_value(b5->toArray(), ([])));
    expectEqual(TEST_LINE, 0, b6->size());
    expectEqual(TEST_LINE, 0, b7->size());
}

private void eachCallsFunctionWithEachElement(void) {
    mixed *calledWith;

    testFunction = new TestFunction();

    a5->each(testFunction, 0, a5->size() - 1);
    calledWith = testFunction->getCalledWith(-1);

    expectEqual(TEST_LINE, 11, testFunction->getCalledTimes());
    expectEqual(TEST_LINE, "({ 1, 0 })", dump_value(calledWith[0], ([ ])));
    expectEqual(TEST_LINE, "({ 2, 1 })", dump_value(calledWith[1], ([ ])));
    expectEqual(TEST_LINE, "({ 2, 2 })", dump_value(calledWith[2], ([ ])));
    expectEqual(TEST_LINE, "({ 3, 3 })", dump_value(calledWith[3], ([ ])));
    expectEqual(TEST_LINE, "({ 4, 4 })", dump_value(calledWith[4], ([ ])));
    expectEqual(TEST_LINE, "({ 4, 5 })", dump_value(calledWith[5], ([ ])));
    expectEqual(TEST_LINE, "({ 5, 6 })", dump_value(calledWith[6], ([ ])));
    expectEqual(TEST_LINE, "({ 6, 7 })", dump_value(calledWith[7], ([ ])));
    expectEqual(TEST_LINE, "({ nil, 8 })", dump_value(calledWith[8], ([ ])));
    expectEqual(TEST_LINE, "({ 6, 9 })", dump_value(calledWith[9], ([ ])));
    expectEqual(TEST_LINE, "({ 7, 10 })", dump_value(calledWith[10], ([ ])));
}

private void eachCallsFunctionWithSubsetOfElements(void) {
    mixed *calledWith;

    testFunction = new TestFunction();

    a5->each(testFunction, 3, 6);
    calledWith = testFunction->getCalledWith(-1);

    expectEqual(TEST_LINE, 4, testFunction->getCalledTimes());
    expectEqual(TEST_LINE, "({ 3, 3 })", dump_value(calledWith[0], ([ ])));
    expectEqual(TEST_LINE, "({ 4, 4 })", dump_value(calledWith[1], ([ ])));
    expectEqual(TEST_LINE, "({ 4, 5 })", dump_value(calledWith[2], ([ ])));
    expectEqual(TEST_LINE, "({ 5, 6 })", dump_value(calledWith[3], ([ ])));
}

private void toBoxReducerDefault(void) {
    string str;
    string *items;

    toBoxReducer = new ToBoxReducer(({ 0, 0, 0, 0 }), ({ 0, 0, 0, 0 }));

    str = "one long str two";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "+----------------+\n" +
                           "|one long str two|\n" +
                           "+----------------+", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "+--------+\n" +
                           "|one     |\n" +
                           "|long str|\n" +
                           "|two     |\n" +
                           "+--------+", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "+--------+\n" +
                           "|one     |\n" +
                           "|long str|\n" +
                           "|two     |\n" +
                           "+--------+", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "+--------+\n" +
                           "|one     |\n" +
                           "|long str|\n" +
                           "|two     |\n" +
                           "+--------+", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "+--------+\n" +
                           "|one     |\n" +
                           "|long str|\n" +
                           "|two     |\n" +
                           "+--------+", new Array(explode(str, "\n"))->box(toBoxReducer));

    toBoxReducer = new ToBoxReducer(({ 2, 2, 2, 2 }), ({ 2, 2, 2, 2 }));

    str = "one long str two";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "\n\n" +
                           "  +--------------------+  \n" +
                           "  |                    |  \n" +
                           "  |                    |  \n" +
                           "  |  one long str two  |  \n" +
                           "  |                    |  \n" +
                           "  |                    |  \n" +
                           "  +--------------------+  \n\n", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "\n\n" +
                           "  +------------+  \n" +
                           "  |            |  \n" +
                           "  |            |  \n" +
                           "  |  one       |  \n" +
                           "  |  long str  |  \n" +
                           "  |  two       |  \n" +
                           "  |            |  \n" +
                           "  |            |  \n" +
                           "  +------------+  \n\n", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "\n\n" +
                           "  +------------+  \n" +
                           "  |            |  \n" +
                           "  |            |  \n" +
                           "  |  one       |  \n" +
                           "  |  long str  |  \n" +
                           "  |  two       |  \n" +
                           "  |            |  \n" +
                           "  |            |  \n" +
                           "  +------------+  \n\n", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "\n\n" +
                           "  +------------+  \n" +
                           "  |            |  \n" +
                           "  |            |  \n" +
                           "  |  one       |  \n" +
                           "  |  long str  |  \n" +
                           "  |  two       |  \n" +
                           "  |            |  \n" +
                           "  |            |  \n" +
                           "  +------------+  \n\n", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "\n\n" +
                           "  +------------+  \n" +
                           "  |            |  \n" +
                           "  |            |  \n" +
                           "  |  one       |  \n" +
                           "  |  long str  |  \n" +
                           "  |  two       |  \n" +
                           "  |            |  \n" +
                           "  |            |  \n" +
                           "  +------------+  \n\n", new Array(explode(str, "\n"))->box(toBoxReducer));
}

private void toBoxReducerSingle(void) {
    string str;
    string *items;

    toBoxReducer = new ToBoxReducer(({ 0, 0, 0, 0 }), ({ 0, 0, 0, 0 }), BOX_STRING_SINGLE);

    str = "one long str two";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "┌────────────────┐\n" +
                           "│one long str two│\n" +
                           "└────────────────┘", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "┌────────┐\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "└────────┘", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "┌────────┐\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "└────────┘", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "┌────────┐\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "└────────┘", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "┌────────┐\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "└────────┘", new Array(explode(str, "\n"))->box(toBoxReducer));
}

private void toBoxReducerDouble(void) {
    string str;
    string *items;

    toBoxReducer = new ToBoxReducer(({ 0, 0, 0, 0 }), ({ 0, 0, 0, 0 }), BOX_STRING_DOUBLE);

    str = "one long str two";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════════════╗\n" +
                           "║one long str two║\n" +
                           "╚════════════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════╗\n" +
                           "║one     ║\n" +
                           "║long str║\n" +
                           "║two     ║\n" +
                           "╚════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════╗\n" +
                           "║one     ║\n" +
                           "║long str║\n" +
                           "║two     ║\n" +
                           "╚════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════╗\n" +
                           "║one     ║\n" +
                           "║long str║\n" +
                           "║two     ║\n" +
                           "╚════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════╗\n" +
                           "║one     ║\n" +
                           "║long str║\n" +
                           "║two     ║\n" +
                           "╚════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));
}

private void toBoxReducerDoubleHorizontal(void) {
    string str;
    string *items;

    toBoxReducer = new ToBoxReducer(({ 0, 0, 0, 0 }), ({ 0, 0, 0, 0 }), BOX_STRING_DOUBLE_HORIZONTAL);

    str = "one long str two";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════════════╗\n" +
                           "│one long str two│\n" +
                           "╚════════════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════╗\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "╚════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════╗\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "╚════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════╗\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "╚════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╔════════╗\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "╚════════╝", new Array(explode(str, "\n"))->box(toBoxReducer));
}

private void toBoxReducerRound(void) {
    string str;
    string *items;

    toBoxReducer = new ToBoxReducer(({ 0, 0, 0, 0 }), ({ 0, 0, 0, 0 }), BOX_STRING_ROUND);

    str = "one long str two";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╭────────────────╮\n" +
                           "│one long str two│\n" +
                           "╰────────────────╯", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╭────────╮\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "╰────────╯", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╭────────╮\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "╰────────╯", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "one\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╭────────╮\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "╰────────╯", new Array(explode(str, "\n"))->box(toBoxReducer));

    str = "\none\nlong str\ntwo\n";
    items = explode(str, "\n");
    expectEqual(TEST_LINE, "╭────────╮\n" +
                           "│one     │\n" +
                           "│long str│\n" +
                           "│two     │\n" +
                           "╰────────╯", new Array(explode(str, "\n"))->box(toBoxReducer));
}

static void runBeforeTests(void) {
    array = new Array(({ -1., 0., 1. }));
    one = new Array(({ 42. }));
    empty = new Array(({ }));
    c = new Array(({ 2., 4., 5., 6., 8. }));
    d = new Array(({ 1., 3., 5., 7., 8. }));

    a0 = new Array(({ }));
    a1 = new Array(({ 1 }));
    a2 = new Array(({ 1, 2 }));
    a3 = new Array(({ 1, 1 }));
    a4 = new Array(({ 1, 2, 2, 3, 4, 4, 5, 6, 6, 7 }));
    a5 = new Array(({ 1, 2, 2, 3, 4, 4, 5, 6, nil, 6, 7 }));
    a6 = new Array(({ nil }));
    a7 = new Array(({ nil, nil }));
}

void runTests(void) {
    toArrayShouldReturnUnderpinningArray();
    sizeShouldReturnSizeOfUnderpinningArray();
    filterShouldFilterFromGivenParameters();
    mapShouldMapFromGivenParameters();
    reduceShouldReduceFromGivenParameters();
    arrayShouldBeIterable();
    arrayShouldBeIndexable();
    arrayIndexAssignment();
    unionShouldCreateUnionOfSets();
    intersectionShouldCreateIntersectionOfSets();
    symmetricDifferenceShouldCreateSymDifOfSets();
    differenceShouldCreateDifferenceOfSets();
    getOutOfRangeShouldThrowExpectedError();
    toMarkedListReducerCreateUnorderedList();
    toMarkedListReducerCreateOrderedList();
    containsShouldIndicateIfArrayContainsValue();
    tabulateShouldCreateTabulatedOutput();
    uniqueShouldCreateUniqueArray();
    eachCallsFunctionWithEachElement();
    eachCallsFunctionWithSubsetOfElements();
    toBoxReducerDefault();
    toBoxReducerSingle();
    toBoxReducerDouble();
    toBoxReducerDoubleHorizontal();
    toBoxReducerRound();
}