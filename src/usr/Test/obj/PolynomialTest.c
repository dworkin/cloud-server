#include <Test.h>

inherit Test;

private Polynomial p;
private Polynomial d;
private ProbabilityReducer reducer;

private void degreeShouldReturnSizeOfMinusOne(void) {
    expectEqual(TEST_LINE, 3, p->degree());
}

private void coefficientsShouldReturnTheCoefficients(void) {
    float *c;

    c = p->coefficients();

    expectEqual(TEST_LINE, 4, sizeof(c));
    expectEqual(TEST_LINE, 0., c[0]);
    expectEqual(TEST_LINE, 1., c[1]);
    expectEqual(TEST_LINE, 2., c[2]);
    expectEqual(TEST_LINE, 3., c[3]);
}

private void evaluateShouldReturnExpectedValue(void) {
    expectEqual(TEST_LINE, p->evaluate(3.), 102.);
}

private void differentiateShouldReturnDerivative(void) {
    Polynomial m;
    float *c;

    c = d->coefficients();

    expectEqual(TEST_LINE, 2, d->degree());
    expectEqual(TEST_LINE, 3, sizeof(c));
    expectEqual(TEST_LINE, 1., c[0]);
    expectEqual(TEST_LINE, 4., c[1]);
    expectEqual(TEST_LINE, "x^0 + 4x + 9x^2", d->toString());

    m = new Polynomial(({ 5. }));
    m = m->differentiate();
    c = m->coefficients();
    expectEqual(TEST_LINE, 0., c[0]);
}

private void toStringShouldReturnStringRepresentation(void) {
    expectEqual(TEST_LINE, "x + 2x^2 + 3x^3", p->toString());
    expectEqual(TEST_LINE, "t + 2t^2 + 3t^3", p->toString("t"));
}

private void equalsShouldReturnExpectedValue(void) {
    expectTrue(TEST_LINE, p->equals(p));
    expectTrue(TEST_LINE, !p->equals(p->differentiate()));
}

private void addShouldAddValue(void) {
    float *c;

    c = (p + d)->coefficients();

    expectEqual(TEST_LINE, 4, sizeof(c));
    expectEqual(TEST_LINE, 1., c[0]);
    expectEqual(TEST_LINE, 5., c[1]);
    expectEqual(TEST_LINE, 11., c[2]);
    expectEqual(TEST_LINE, 3., c[3]);
}

private void subtractShouldSubtractValue(void) {
    float *c;

    c = (p - d)->coefficients();

    expectEqual(TEST_LINE, 4, sizeof(c));
    expectEqual(TEST_LINE, -1., c[0]);
    expectEqual(TEST_LINE, -3., c[1]);
    expectEqual(TEST_LINE, -7., c[2]);
    expectEqual(TEST_LINE, 3., c[3]);
}

private void multiplyShouldMultiplyValue(void) {
    Polynomial m;
    float *c;

    m = p * d;
    c = m->coefficients();

    expectEqual(TEST_LINE, 6, sizeof(c));
    expectEqual(TEST_LINE, 0., c[0]);
    expectEqual(TEST_LINE, 1., c[1]);
    expectEqual(TEST_LINE, 6., c[2]);
    expectEqual(TEST_LINE, 20., c[3]);
    expectEqual(TEST_LINE, 30., c[4]);
    expectEqual(TEST_LINE, 27., c[5]);
}

private void indexShouldReturnExpectedValue(void) {
    Polynomial m;
    float f;
    float *c;
    string e;

    f = p[1];

    expectEqual(TEST_LINE, 1., f);

    m = p[1..2];
    c = m->coefficients();

    expectEqual(TEST_LINE, 2, sizeof(c));
    expectEqual(TEST_LINE, 1., c[0]);
    expectEqual(TEST_LINE, 2., c[1]);

    m = p[0..];
    c = m->coefficients();

    expectEqual(TEST_LINE, 4, sizeof(c));
    expectEqual(TEST_LINE, 0., c[0]);
    expectEqual(TEST_LINE, 1., c[1]);
    expectEqual(TEST_LINE, 2., c[2]);
    expectEqual(TEST_LINE, 3., c[3]);

    m = p[..2];
    c = m->coefficients();

    expectEqual(TEST_LINE, 3, sizeof(c));
    expectEqual(TEST_LINE, 0., c[0]);
    expectEqual(TEST_LINE, 1., c[1]);
    expectEqual(TEST_LINE, 2., c[2]);

    m = p[..];
    c = m->coefficients();

    expectEqual(TEST_LINE, 4, sizeof(c));
    expectEqual(TEST_LINE, 0., c[0]);
    expectEqual(TEST_LINE, 1., c[1]);
    expectEqual(TEST_LINE, 2., c[2]);
    expectEqual(TEST_LINE, 3., c[3]);
}

private void indexEqualShouldUpdateValue(void) {
    Polynomial m;
    float f;

    m = new Polynomial(p->coefficients());
    m[1] = 2.;
    f = m[1];

    expectEqual(TEST_LINE, 2., f);
}

private void createShouldHandleParameters(void) {
    Polynomial m;
    float *c;

    expectEqual(TEST_LINE, "Insufficient arguments for function create", catch(new Polynomial()));
    expectEqual(TEST_LINE, "Polynomial: parameters cannot be empty.", catch(new Polynomial(({}))));

    m = new Polynomial(({ 0. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "", m->toString());
    expectEqual(TEST_LINE, 1, sizeof(c));
    expectEqual(TEST_LINE, 0., c[0]);

    m = new Polynomial(({ 1. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "x^0", m->toString());
    expectEqual(TEST_LINE, 1, sizeof(c));
    expectEqual(TEST_LINE, 1., c[0]);

    m = new Polynomial(({ 2. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "2x^0", m->toString());
    expectEqual(TEST_LINE, 1, sizeof(c));
    expectEqual(TEST_LINE, 2., c[0]);

    m = new Polynomial(({ 0., 0. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "", m->toString());
    expectEqual(TEST_LINE, 2, sizeof(c));
    expectEqual(TEST_LINE, 0., c[0]);
    expectEqual(TEST_LINE, 0., c[1]);

    m = new Polynomial(({ 1., 0. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "x^0", m->toString());
    expectEqual(TEST_LINE, 2, sizeof(c));
    expectEqual(TEST_LINE, 1., c[0]);
    expectEqual(TEST_LINE, 0., c[1]);

    m = new Polynomial(({ 0., 1. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "x", m->toString());
    expectEqual(TEST_LINE, 2, sizeof(c));
    expectEqual(TEST_LINE, 0., c[0]);
    expectEqual(TEST_LINE, 1., c[1]);

    m = new Polynomial(({ 1., 1. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "x^0 + x", m->toString());
    expectEqual(TEST_LINE, 2, sizeof(c));
    expectEqual(TEST_LINE, 1., c[0]);
    expectEqual(TEST_LINE, 1., c[1]);

    m = new Polynomial(({ 2., 0. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "2x^0", m->toString());
    expectEqual(TEST_LINE, 2, sizeof(c));
    expectEqual(TEST_LINE, 2., c[0]);
    expectEqual(TEST_LINE, 0., c[1]);

    m = new Polynomial(({ 0., 2. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "2x", m->toString());
    expectEqual(TEST_LINE, 2, sizeof(c));
    expectEqual(TEST_LINE, 0., c[0]);
    expectEqual(TEST_LINE, 2., c[1]);

    m = new Polynomial(({ 2., 2. }));
    c = m->coefficients();
    expectEqual(TEST_LINE, "2x^0 + 2x", m->toString());
    expectEqual(TEST_LINE, 2, sizeof(c));
    expectEqual(TEST_LINE, 2., c[0]);
    expectEqual(TEST_LINE, 2., c[1]);
}

private void derivativeIteratorShouldIterateFromDegree(void) {
    Iterator iterator;

    iterator = new PolynomialDerivativeIterator(p);

    expectTrue(TEST_LINE, !iterator->end());
    expectEqual(TEST_LINE, 3., iterator->next());
    expectEqual(TEST_LINE, 2., iterator->next());
    expectEqual(TEST_LINE, 2., iterator->next());
    expectTrue(TEST_LINE, iterator->end());
    expectEqual(TEST_LINE, 0., iterator->next());
}

private void stringReducerShouldReducePolynomial(void) {
    Function stringReducer;
    string a, b, c, d, e, f;
    stringReducer = new PolynomialStringReducer("y");

    a = stringReducer->evaluate(3., 2);
    b = stringReducer->evaluate(4., 3);
    c = stringReducer->evaluate(0., 1);
    d = stringReducer->evaluate(0., 2);
    e = stringReducer->evaluate(1., 0);
    f = stringReducer->evaluate(2., 0);

    expectEqual(TEST_LINE, "3y^2", a);
    expectEqual(TEST_LINE, " + 4y^3", b);
    expectEqual(TEST_LINE, "", c);
    expectEqual(TEST_LINE, "", d);
    expectEqual(TEST_LINE, " + y^0", e);
    expectEqual(TEST_LINE, " + 2y^0", f);
}

private void integrateShouldReturnExpectedValue(void) {
    expectEqual(TEST_LINE, 68., d->integrate(2., 3.));
}

private void applyImpossibleLowerCase(void) {
    mapping actual;
    reducer->apply(1);

    actual = reducer->evaluate();

    expectEqual(TEST_LINE, 0., actual["<"]);
    expectEqual(TEST_LINE, 0., actual["="]);
    expectEqual(TEST_LINE, 1., actual[">"]);
}

private void applyImpossibleUpperCase(void) {
    mapping actual;
    reducer->apply(100);

    actual = reducer->evaluate();

    expectEqual(TEST_LINE, 1., actual["<"]);
    expectEqual(TEST_LINE, 0., actual["="]);
    expectEqual(TEST_LINE, 0., actual[">"]);
}

private void applyMinEdgeCase(void) {
    mapping actual;
    reducer->apply(3);

    actual = reducer->evaluate();

    expectEqual(TEST_LINE, 0., actual["<"]);
    expectEqual(TEST_LINE, 7.71604938e-4, actual["="]);
    expectEqual(TEST_LINE, 1., actual[">"]);
}

private void applyTen(void) {
    mapping actual;
    reducer->apply(10);

    actual = reducer->evaluate();

    expectEqual(TEST_LINE, 0.175154321, actual["<"]);
    expectEqual(TEST_LINE, 0.0941358025, actual["="]);
    expectEqual(TEST_LINE, 0.730709877, actual[">"]);
}

private void applyMaxEdgeCase(void) {
    mapping actual;
    reducer->apply(18);

    actual = reducer->evaluate();

    expectEqual(TEST_LINE, 1., actual["<"]);
    expectEqual(TEST_LINE, 0.0162037037, actual["="]);
    expectEqual(TEST_LINE, 0., actual[">"]);
}

private void verifyTestPolynomial(void) {
    Polynomial e;

    e = new Polynomial(
            ({ 0., 0., 0., 1., 4., 10., 21., 38., 62., 91., 122., 148., 167., 172., 160., 131., 94., 54., 21. })
    );

    reducer = new ProbabilityReducer(e, 6, 4);

    expectEqual(TEST_LINE,
                "x^3 + 4x^4 + 10x^5 + 21x^6 + 38x^7 + 62x^8 + 91x^9 + 122x^10 + 148x^11 + 167x^12 + 172x^13 + 160x^14 " +
                "+ 131x^15 + 94x^16 + 54x^17 + 21x^18",
                e->toString());
}

static void runBeforeTests(void) {
    p = new Polynomial(({ 0., 1., 2., 3., }));
    d = p->differentiate();
}

void runTests(void) {
    degreeShouldReturnSizeOfMinusOne();
    coefficientsShouldReturnTheCoefficients();
    evaluateShouldReturnExpectedValue();
    differentiateShouldReturnDerivative();
    toStringShouldReturnStringRepresentation();
    equalsShouldReturnExpectedValue();
    addShouldAddValue();
    subtractShouldSubtractValue();
    multiplyShouldMultiplyValue();
    indexShouldReturnExpectedValue();
    indexEqualShouldUpdateValue();
    createShouldHandleParameters();
    derivativeIteratorShouldIterateFromDegree();
    stringReducerShouldReducePolynomial();
    integrateShouldReturnExpectedValue();
    verifyTestPolynomial();
    applyImpossibleLowerCase();
    applyImpossibleUpperCase();
    applyMinEdgeCase();
    applyTen();
    applyMaxEdgeCase();
}
