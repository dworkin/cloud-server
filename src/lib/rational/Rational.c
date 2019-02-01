#include <NKlib.h>

inherit Number;
inherit NK_KFUN;

private int numerator;
private int denominator;

private void fixSign(void) {
    if (denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }
}

private void toCanonical(void) {
    int g;
    g = gcd(numerator, denominator);
    numerator /= g;
    denominator /= g;
}

int numerator(void) {
    return numerator;
}

int denominator(void) {
    return denominator;
}

void fromRatio(int numerator, int denominator) {
    ::numerator = numerator;
    ::denominator = denominator;
    if (!denominator) {
        error("Rational: the denominator must be different from zero.");
    }
    fixSign();
    if (numerator != 0) {
        toCanonical();
    }
}

void fromFloat(float f) {
    float fCopy, fCopyFloor, n0, n1, n2, d0, d1, d2;
    float oneOver, oneOverFloor;
    int n;

    if (f == 1.) {
        numerator = 1;
        denominator = 1;
        return;
    }

    if (f == 0.) {
        numerator = 0;
        denominator = 1;
        return;
    }

    fCopy = f;
    fCopyFloor = floor(fCopy);
    n0 = 1.;
    n1 = fCopyFloor;
    n2 = 0.;
    d0 = 0.;
    d1 = 1.;
    d2 = 1.;

    if (fabs(fCopyFloor - f) <= TOLERANCE) {
        numerator = (int) fCopyFloor;
        denominator = 1;
        return;
    }

    for (n = 0; n < MAX_ITERATIONS; n++) {
        oneOver = 1. / (fCopy - fCopyFloor);
        oneOverFloor = floor(oneOver);
        n2 = oneOverFloor * n1 + n0;
        d2 = oneOverFloor * d1 + d0;

        if (fabs(n2 / d2 -f) <= TOLERANCE) {
            break;
        }

        n0 = n1;
        n1 = n2;
        d0 = d1;
        d1 = d2;
        fCopyFloor = oneOverFloor;
        fCopy = oneOver;
    }

    numerator = (int) n2;
    denominator = (int) d2;
}

void fromRational(Rational r) {
    fromRatio(r->numberator(), r->denominator());
}

void reciprocal(void) {
    int tmp;

    if (numerator == 0) {
        error("Rational: the operation would result in a denominator of zero.");
    }

    tmp = denominator;
    denominator = numerator;
    numerator = tmp;
    fixSign();
    toCanonical();
}

private void instanceOfBadType(mixed x) {
    error("Rational: cannot operate with value of type " + typeof(x));
}

private Rational mixedToRational(mixed x) {
    float f;

    switch (typeof(x)) {
        case T_FLOAT:
            return new Rational(x);
        case T_INT:
            return new Rational(x, 1);
        case T_STRING:
            if (catch(f = (float) x)) {
                instanceOfBadType(x);
            }
            return new Rational(f);
        case T_OBJECT:
            if (x <- NUMBER_LIB) {
                return x->toRational();
            }
            break;
        default:
            instanceOfBadType(x);
    }

    if (catch(x <- RATIONAL_LIB)) {
        instanceOfBadType(x);
    }

    return x;
}

private int *makeCommon(Rational r) {
    int n1, n2, d1;

    n1 = numerator;
    d1 = denominator;
    n2 = r->numerator();

    n1 *= r->denominator();
    d1 *= r->denominator();
    n2 *= denominator;

    return ({ n1, n2, d1 });
}

static Rational operator+ (mixed r) {
    int n1, n2, d1;

    ({ n1, n2, d1 }) = makeCommon(mixedToRational(r));

    return new Rational(n1 + n2, d1);
}

static Rational operator- (mixed r) {
    int n1, n2, d1;

    ({ n1, n2, d1 }) = makeCommon(mixedToRational(r));

    return new Rational(n1 - n2, d1);
}

static Rational operator^ (mixed p) {
    int i;

    switch (typeof(p)) {
        case T_OBJECT:
            if (p <- NUMBER_LIB) {
                i = p->toInt();
                break;
            }
            error("Rational: invalid power.");
        case T_INT:
        case T_FLOAT:
            i = (int) p;
            break;
        break;
        case T_STRING:
            if (catch(i = (int)(float) p)) {
                error("Rational: invalid power.");
            }
            break;
        default:
            error("Rational: invalid power.");
    }

    if (i == 0) {
        return new Rational(1, 1);
    }

    return new Rational((int) pow((float) numerator, (float) i), (int) pow((float) denominator, (float) i));
}

static Rational operator* (mixed r) {
    r = mixedToRational(r);

    return new Rational(numerator * r->numerator(), denominator * r->denominator());
}

static Rational operator/ (mixed r) {
    Rational rr;

    r = mixedToRational(r);
    rr = new Rational(r->numerator(), r->denominator());
    rr->reciprocal();

    return new Rational(numerator, denominator) * rr;
}

static int operator< (mixed r) {
    int n1, n2;

    ({ n1, n2 }) = makeCommon(mixedToRational(r));

    return n1 < n2;
}

static int operator<= (mixed r) {
    int n1, n2;

    ({ n1, n2 }) = makeCommon(mixedToRational(r));

    return n1 <= n2;
}

static int operator> (mixed r) {
    int n1, n2;

    ({ n1, n2 }) = makeCommon(mixedToRational(r));

    return n1 > n2;
}

static int operator>= (mixed r) {
    int n1, n2;

    ({ n1, n2 }) = makeCommon(mixedToRational(r));

    return n1 >= n2;
}

int equals(mixed r) {
    int n1, n2;

    ({ n1, n2 }) = makeCommon(mixedToRational(r));

    return n1 == n2;
}

string toString(void) {
    return "" + numerator + "/" + denominator;
}

float toFloat(void) {
    return (float) numerator / (float) denominator;
}

int toInt(void) {
    return numerator / denominator;
}

Rational toRational(void) {
    return new Rational(numerator, denominator);
}

static void create(mixed args...) {
    if (sizeof(args) == 1) {
        if (T_FLOAT == typeof(args[0])) {
            if (modf(args[0])[0] == 0.) {
                fromRatio((int) args[0], 1);
                return;
            }
            fromFloat(args[0]);
            return;
        }

        if (T_OBJECT == typeof(args[0])) {
            if (args[0] <- RATIONAL_LIB) {
                fromRational(args[0]);
                return;
            }
            if (args[0] <- NUMBER_LIB) {
                fromRational(args[0]->toRational());
                return;
            }
        }
    } else if (sizeof(args) == 2 && T_INT == typeof(args[0]) && T_INT == typeof(args[1])) {
        fromRatio(args[0], args[1]);
        return;
    }

    error("Rational: invalid parameters.");
}
