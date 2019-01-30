#include <NKlib.h>

inherit Function;
inherit NK_KFUN;

private float *coeffs;

static void create(float *coeffs) {
    if (!sizeof(coeffs)) {
        error("Polynomial: parameters cannot be empty.");
    }

    ::coeffs = coeffs;
}

int degree(void) {
    return sizeof(coeffs) - 1;
}

float *coefficients(void) {
    return coeffs;
}

float evaluate(float at) {
    int i;
    float total;
    Iterator iterator;

    iterator = new IntIterator(0, sizeof(coeffs) - 1);
    i = 0;
    total = 0.;
    while (!iterator->end()) {
        i = iterator->next();
        total += coeffs[i] * pow(at, (float) i);
    }

    return total;
}

float integrate(float from, float to, varargs Integrator integrator) {
    if (!integrator) {
        integrator = new SimpsonsRuleIntegrator(this_object());
    }
    return integrator->integrate(from, to);
}

Polynomial differentiate(void) {
    int i, sz;
    float *f;
    Iterator iterator;

    sz = sizeof(coeffs);
    if (sz == 1) {
	return new Polynomial(({ 0. }));
    }

    i = degree();
    f = allocate_float(i);
    iterator = new PolynomialDerivativeIterator(this_object());

    while (!iterator->end()) {
        f[i - 1] = (float) i * iterator->next();
        i--;
    }

    return new Polynomial(f);
}

string toString(varargs string parameter) {
    Function reducer;
    Iterator iterator;
    int i;
    string str;

    if (!parameter) {
        parameter = "x";
    }

    reducer = new PolynomialStringReducer(parameter);
    iterator = new IntIterator(0, sizeof(coeffs) - 1);
    i = 0;
    str = "";
    while (!iterator->end()) {
        i = iterator->next();
        str += reducer->evaluate(coeffs[i], i);
    }

    return str;
}

int equals(Polynomial p) {
    float *f1, *f2;
    int i, sz;

    if (degree() != p->degree()) {
        return FALSE;
    }

    f1 = coeffs;
    f2 = p->coefficients();
    for (i = 0, sz = sizeof(f1); i < sz; i++) {
        if (f1[i] != f2[i]) {
            return FALSE;
        }
    }

    return TRUE;
}

private Polynomial addSubtractOperation(Polynomial p, int x) {
    float *largest, *newCoeffs;
    int minDegree, maxDegree, i;

    if (x > 0) {
        x = 1;
    }

    if (x < 0) {
        x = -1;
    }

    minDegree = min(degree(), p->degree()) + 1;
    maxDegree = max(degree(), p->degree()) + 1;
    newCoeffs = allocate_float(maxDegree);

    if (degree() < p->degree()) {
        largest = p->coefficients();
    } else {
        largest = coefficients();
    }

    for (i = 0; i < minDegree; i++) {
        newCoeffs[i] = coefficients()[i] + ((float)x * p->coefficients()[i]);
    }

    for (; i < maxDegree; i++) {
        newCoeffs[i] = largest[i];
    }

    return new Polynomial(newCoeffs);
}

static Polynomial operator+ (Polynomial p) {
    return addSubtractOperation(p, 1);
}

static Polynomial operator- (Polynomial p) {
    return addSubtractOperation(p, -1);
}

static Polynomial operator* (Polynomial p) {
    float *newCoeffs, *pCoeffs;
    int i, j, m, n, o;

    pCoeffs = p->coefficients();
    m = sizeof(coeffs);
    n = sizeof(pCoeffs);
    o = m + n - 1;
    newCoeffs = allocate_float(m + n - 1);

    for (i = 0; i < o; i++) {
        newCoeffs[i] = 0.;
        for (j = max(0, 1 - n + i); j < min(m, i + 1); j++) {
            newCoeffs[i] += coeffs[j] * pCoeffs[i - j];
        }
    }

    return new Polynomial(newCoeffs);
}

static float operator[] (int index) {
    return coeffs[index];
}

static void operator[]= (int index, float coeff) {
    coeffs[index] = coeff;
}

static Polynomial operator[..] (mixed from, mixed to) {
    if (from == nil) {
        from = 0;
    }

    if (to == nil) {
        to = degree();
    }

    return new Polynomial(coeffs[from..to]);
}
