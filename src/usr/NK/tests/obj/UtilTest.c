#include <NKlib.h>

inherit Test;
inherit NK_KFUN;

private CheckEquals checkEquals;
private Number number;
private Random random;
private Terminal terminal;

private void piShouldReturnExpectedValue(void) {
    expectEqual(TEST_LINE, 3.14159265, pi());
}

private void minShouldReturnMinimumValue(void) {
    Rational r1, r2;

    r1 = new Rational(0.2);
    r2 = new Rational(0.4);

    expectEqual(TEST_LINE, 1, min(2, 1));
    expectEqual(TEST_LINE, 1, min(1, 2));
    expectEqual(TEST_LINE, 1., min(2., 1.));
    expectEqual(TEST_LINE, 1., min(1., 2.));
    expectEqual(TEST_LINE, "ABC", min("ABC", "abc"));
    expectEqual(TEST_LINE, "ABC", min("abc", "ABC"));
    expectTrue(TEST_LINE, r1->equals(min(r1, r2)));
    expectTrue(TEST_LINE, r1->equals(min(r2, r1)));
}

private void maxShouldReturnMaximumValue(void) {
    Rational r1, r2;

    r1 = new Rational(0.2);
    r2 = new Rational(0.4);

    expectEqual(TEST_LINE, 2, max(2, 1));
    expectEqual(TEST_LINE, 2, max(1, 2));
    expectEqual(TEST_LINE, 2., max(2., 1.));
    expectEqual(TEST_LINE, 2., max(1., 2.));
    expectEqual(TEST_LINE, "abc", max("ABC", "abc"));
    expectEqual(TEST_LINE, "abc", max("abc", "ABC"));
    expectTrue(TEST_LINE, r2->equals(max(r1, r2)));
    expectTrue(TEST_LINE, r2->equals(max(r2, r1)));
}

private void checkEqualsShouldIndicateWhetherOneEqualsAnother(void) {
    string str;
    int i;
    float f;

    str = random->randomString(42);
    i = random->randomInt() % 42;
    number = new Number(i);

    expectTrue(TEST_LINE, checkEquals->checkEquals(number, number));
    expectTrue(TEST_LINE, checkEquals->checkEquals(i, i));
    expectTrue(TEST_LINE, checkEquals->checkEquals((float) i, (float) i));
    expectTrue(TEST_LINE, checkEquals->checkEquals(str, str));

    expectFalse(TEST_LINE, checkEquals->checkEquals(number, new Number(1)));
    expectFalse(TEST_LINE, checkEquals->checkEquals(i, i + 1));
    expectFalse(TEST_LINE, checkEquals->checkEquals((float) i, (float) i + 0.5));
    expectFalse(TEST_LINE, checkEquals->checkEquals(str, "not " + str));
    expectFalse(TEST_LINE, checkEquals->checkEquals(str, i));
    expectFalse(TEST_LINE, checkEquals->checkEquals(str, (float) i));
    expectFalse(TEST_LINE, checkEquals->checkEquals(str, number));
    expectFalse(TEST_LINE, checkEquals->checkEquals(i, (float) i));
    expectFalse(TEST_LINE, checkEquals->checkEquals(number, str));
    expectFalse(TEST_LINE, checkEquals->checkEquals(number, i));
    expectFalse(TEST_LINE, checkEquals->checkEquals(number, (float) i));

    expectFalse(TEST_LINE, checkEquals->checkEquals(number, ({ })));
    expectFalse(TEST_LINE, checkEquals->checkEquals(number, ([ ])));
    expectFalse(TEST_LINE, checkEquals->checkEquals(i, ({ })));
    expectFalse(TEST_LINE, checkEquals->checkEquals(i, ([ ])));
    expectFalse(TEST_LINE, checkEquals->checkEquals((float) i, ({ })));
    expectFalse(TEST_LINE, checkEquals->checkEquals((float) i, ([ ])));
    expectFalse(TEST_LINE, checkEquals->checkEquals(str, ({ })));
    expectFalse(TEST_LINE, checkEquals->checkEquals(str, ([ ])));
}

private void checkNumberReturnsExpectedValue(void) {
    number = new Number(0.8);

    expectEqual(TEST_LINE, "0.8", number->toString());
    expectEqual(TEST_LINE, 1, number->toInt());
    expectEqual(TEST_LINE, 0, new Number(0.2)->toInt());
    expectEqual(TEST_LINE, 0.8, number->toFloat());
    expectTrue(TEST_LINE, new Rational(4, 5)->equals(number->toRational()));
}

private void colors256ShouldThrowExpectedError(void) {
    expectEqual(TEST_LINE, "Terminal: Invalid color.", catch(terminal->color256(-1, "foo")));
    expectEqual(TEST_LINE, "Terminal: Invalid color.", catch(terminal->color256(256, "foo")));
}

void runBeforeTests(void) {
    checkEquals = new CheckEquals();
    random = new Random();
    terminal = new Terminal();
}

void runTests(void) {
    piShouldReturnExpectedValue();
    minShouldReturnMinimumValue();
    maxShouldReturnMaximumValue();
    checkEqualsShouldIndicateWhetherOneEqualsAnother();
    checkNumberReturnsExpectedValue();
    colors256ShouldThrowExpectedError();
}
