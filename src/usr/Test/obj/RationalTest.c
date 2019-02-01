#include <Test.h>

inherit Test;

private Rational p;
private Rational q;
private int x;
private float y;
private Number n;

private void addRationals(void) {
    expectEqual(TEST_LINE, "22/15", (p + q)->toString());
    expectEqual(TEST_LINE, "11/3", (p + x)->toString());
    expectEqual(TEST_LINE, "3/2", (p + y)->toString());
    expectEqual(TEST_LINE, "3/2", (p + n)->toString());
    expectEqual(TEST_LINE,
            "Rational: cannot operate with value of type " + T_ARRAY,
            catch((p + ({ }))->toString()));
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun +",
                catch(p + nil));
}

private void subtractRationals(void) {
    expectEqual(TEST_LINE, "-2/15", (p - q)->toString());
    expectEqual(TEST_LINE, "-7/3", (p - x)->toString());
    expectEqual(TEST_LINE, "-1/6", (p - y)->toString());
    expectEqual(TEST_LINE, "-1/6", (p - n)->toString());
    expectEqual(TEST_LINE,
                "Rational: cannot operate with value of type " + T_ARRAY,
                catch((p - ({ }))->toString()));
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun -",
                catch((p - nil)->toString()));
}

private void multiplyRationals(void) {
    expectEqual(TEST_LINE, "8/15", (p * q)->toString());
    expectEqual(TEST_LINE, "2/1", (p * x)->toString());
    expectEqual(TEST_LINE, "5/9", (p * y)->toString());
    expectEqual(TEST_LINE, "5/9", (p * n)->toString());
    expectEqual(TEST_LINE,
                "Rational: cannot operate with value of type " + T_ARRAY,
                catch((p * ({ }))->toString()));
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun *",
                catch((p * nil)->toString()));
}

private void divideRationals(void) {
    expectEqual(TEST_LINE, "5/6", (p / q)->toString());
    expectEqual(TEST_LINE, "2/9", (p / x)->toString());
    expectEqual(TEST_LINE, "4/5", (p / y)->toString());
    expectEqual(TEST_LINE, "4/5", (p / n)->toString());
    expectEqual(TEST_LINE,
                "Rational: cannot operate with value of type " + T_ARRAY,
                catch((p / ({ }))->toString()));
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun /",
                catch((p / nil)->toString()));
}

private void powerRationals(void) {
    int x;

    x = 3;

    expectEqual(TEST_LINE, "8/27", (p ^ 3)->toString());
    expectEqual(TEST_LINE, "8/27", (p ^ 3.)->toString());
    expectEqual(TEST_LINE, "8/27", (p ^ 3.14)->toString());
    expectEqual(TEST_LINE, "2/3", (p ^ 0.8)->toString());
    expectEqual(TEST_LINE, "1/1", (p ^ 0.4)->toString());
    expectEqual(TEST_LINE, "8/27", (p ^ "3")->toString());
    expectEqual(TEST_LINE, "8/27", (p ^ "3.14")->toString());
    expectEqual(TEST_LINE, "8/27", (p ^ new Number(3))->toString());
    expectEqual(TEST_LINE, "8/27", (p ^ new Number(3.))->toString());
    expectEqual(TEST_LINE, "8/27", (p ^ new Number(3.14))->toString());
}

private void lessThanRationals(void) {
    expectTrue(TEST_LINE, p < q);
    expectTrue(TEST_LINE, p < x);
    expectTrue(TEST_LINE, p < y);
    expectEqual(TEST_LINE,
            "Bad argument 2 for kfun <",
            catch(p < nil));
    expectFalse(TEST_LINE, q < p);
    expectTrue(TEST_LINE, q < x);
    expectTrue(TEST_LINE, q < y);
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun <",
                catch(q < nil));
}

private void lessThanOrEqualRationals(void) {
    expectTrue(TEST_LINE, p <= q);
    expectTrue(TEST_LINE, p <= x);
    expectTrue(TEST_LINE, p <= y);
    expectTrue(TEST_LINE, p <= p->toFloat());
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun <=",
    catch(p <= nil));
    expectFalse(TEST_LINE, q <= p);
    expectTrue(TEST_LINE, q <= x);
    expectTrue(TEST_LINE, q <= y);
    expectTrue(TEST_LINE, q <= q->toFloat());
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun <=",
    catch(q <= nil));
}

private void greaterThanRationals(void) {
    expectFalse(TEST_LINE, p > q);
    expectTrue(TEST_LINE, p < x);
    expectTrue(TEST_LINE, p < y);
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun >",
                catch(p > nil));
    expectTrue(TEST_LINE, q > p);
    expectFalse(TEST_LINE, q > x);
    expectFalse(TEST_LINE, q > y);
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun >",
                catch(q > nil));
}

private void greaterThanOrEqualRationals(void) {
    expectFalse(TEST_LINE, p >= q);
    expectFalse(TEST_LINE, p >= x);
    expectFalse(TEST_LINE, p >= y);
    expectTrue(TEST_LINE, p >= p->toFloat());
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun >=",
    catch(p >= nil));
    expectTrue(TEST_LINE, q >= p);
    expectFalse(TEST_LINE, q >= x);
    expectFalse(TEST_LINE, q >= y);
    expectTrue(TEST_LINE, q >= q->toFloat());
    expectEqual(TEST_LINE,
                "Bad argument 2 for kfun >=",
    catch(q >= nil));
}

private void equalToRationals(void) {
    expectTrue(TEST_LINE, p->equals(p));
    expectFalse(TEST_LINE, p->equals(x));
    expectFalse(TEST_LINE, p->equals(y));
    expectTrue(TEST_LINE, p->equals(p->toFloat()));
    expectEqual(TEST_LINE,
                "Rational: cannot operate with value of type " + T_ARRAY,
                catch(p->equals(({ }))));

    expectTrue(TEST_LINE, q->equals(q));
    expectFalse(TEST_LINE, q->equals(x));
    expectFalse(TEST_LINE, q->equals(y));
    expectTrue(TEST_LINE, q->equals(q->toFloat()));
    expectEqual(TEST_LINE,
                "Rational: cannot operate with value of type " + T_ARRAY,
                catch(q->equals(({ }))));

    expectFalse(TEST_LINE, p->equals(q));
    expectFalse(TEST_LINE, p->equals(3));
    expectFalse(TEST_LINE, p->equals(q->toFloat()));

    expectFalse(TEST_LINE, q->equals(p));
    expectFalse(TEST_LINE, q->equals(3));
    expectFalse(TEST_LINE, q->equals(p->toFloat()));
}

private void displayRationalFromRatio(void) {
    expectEqual(TEST_LINE, "4/5", new Rational(4, 5)->toString());
    expectEqual(TEST_LINE, "1/3", new Rational(3, 9)->toString());
}

private void displayRationalFromFloat(void) {
    expectEqual(TEST_LINE, "4/5", new Rational(0.8)->toString());
    expectEqual(TEST_LINE, "83/100", new Rational(0.83)->toString());
    expectEqual(TEST_LINE, "833/1000", new Rational(0.833)->toString());
    expectEqual(TEST_LINE, "8333/10000", new Rational(0.8333)->toString());
    expectEqual(TEST_LINE, "41664/49997", new Rational(0.83333)->toString());
    expectEqual(TEST_LINE, "416669/500003", new Rational(0.833333)->toString());
    expectEqual(TEST_LINE, "4166434/4999721", new Rational(0.8333333)->toString());
    expectEqual(TEST_LINE, "5/6", new Rational(0.83333333)->toString());
    expectEqual(TEST_LINE, "5/6", new Rational(0.833333333)->toString());
    expectEqual(TEST_LINE, "19601/13860", new Rational(sqrt(2.))->toString());
    expectEqual(TEST_LINE, "130/41", new Rational(130./41.)->toString());
}

private void toFloatGivesExpectedValues(void) {
    expectEqual(TEST_LINE, 0.8, new Rational(0.8)->toFloat());
    expectEqual(TEST_LINE, 0.83, new Rational(0.83)->toFloat());
    expectEqual(TEST_LINE, 0.833, new Rational(0.833)->toFloat());
    expectEqual(TEST_LINE, 0.8333, new Rational(0.8333)->toFloat());
    expectEqual(TEST_LINE, 0.83333, new Rational(0.83333)->toFloat());
    expectEqual(TEST_LINE, 0.833333, new Rational(0.833333)->toFloat());
    expectEqual(TEST_LINE, 0.8333333, new Rational(0.8333333)->toFloat());
    expectEqual(TEST_LINE, 0.83333333, new Rational(0.83333333)->toFloat());
    expectEqual(TEST_LINE, 1.41421356, new Rational(sqrt(2.))->toFloat());
    expectEqual(TEST_LINE, 3.17073171, new Rational(130./41.)->toFloat());
    expectEqual(TEST_LINE, 0.8, new Rational(4, 5)->toFloat());
    expectEqual(TEST_LINE, 0.333333333, new Rational(3, 9)->toFloat());
    expectEqual(TEST_LINE, 1.41421356, new Rational(19601, 13860)->toFloat());
    expectEqual(TEST_LINE, 0.8333333, new Rational(4166434, 4999721)->toFloat());
    expectEqual(TEST_LINE, 0.833333, new Rational(416669, 500003)->toFloat());
    expectEqual(TEST_LINE, 0.83333, new Rational(41664, 49997)->toFloat());
    expectEqual(TEST_LINE, 0.8333, new Rational(8333, 10000)->toFloat());
    expectEqual(TEST_LINE, 0.8, new Rational(4, 5)->toFloat());
    expectEqual(TEST_LINE, 0.83333333, new Rational(5, 6)->toFloat());
    expectEqual(TEST_LINE, 0.833333333, new Rational(5, 6)->toFloat());
}

private void createGivesExpectedResults(void) {
    expectEqual(TEST_LINE, "Rational: invalid parameters.", catch(new Rational()));
    expectEqual(TEST_LINE, "Rational: invalid parameters.", catch(new Rational("string")));
    expectEqual(TEST_LINE, "Rational: invalid parameters.", catch(new Rational(({ "string" }))));
    expectEqual(TEST_LINE, "Rational: invalid parameters.", catch(new Rational(({ 42., 42. }))));
    expectEqual(TEST_LINE, "Rational: invalid parameters.", catch(new Rational(({ 42, 42. }))));
    expectEqual(TEST_LINE, "Rational: invalid parameters.", catch(new Rational(1, 2, 3)));
    expectEqual(TEST_LINE, "Rational: invalid parameters.", catch(new Rational(({ 1, 2, 3 }))));
    expectEqual(TEST_LINE, "Rational: the denominator must be different from zero.", catch(new Rational(1, 0)));
}

private void powerErrorsAsExpected(void) {
    expectEqual(TEST_LINE, "Rational: invalid power.", catch(p ^ "str"));
    expectEqual(TEST_LINE, "Rational: invalid power.", catch(p ^ new String("")));
    expectEqual(TEST_LINE, "Rational: invalid power.", catch(p ^ ({ 1 })));
}

private void reciprocalWorksAsExpected(void) {
    Rational r;

    expectEqual(TEST_LINE, "Rational: the operation would result in a denominator of zero.",
            catch(new Rational(0, 2)->reciprocal()));

    r = new Rational(3, 9);
    r->reciprocal();
    expectTrue(TEST_LINE, r->equals(new Rational(9, 3)));
}

void runBeforeTests(void) {
    p = new Rational(2, 3);
    q = new Rational(4, 5);
    x = 3;
    y = 0.83333333;
    n = new Number(y);
}

void runTests(void) {
    addRationals();
    subtractRationals();
    multiplyRationals();
    divideRationals();
    powerRationals();
    lessThanRationals();
    lessThanOrEqualRationals();
    greaterThanRationals();
    greaterThanOrEqualRationals();
    equalToRationals();
    displayRationalFromRatio();
    displayRationalFromFloat();
    toFloatGivesExpectedValues();
    createGivesExpectedResults();
    powerErrorsAsExpected();
    reciprocalWorksAsExpected();
}