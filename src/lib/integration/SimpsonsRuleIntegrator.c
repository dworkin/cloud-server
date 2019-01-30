#include <NKlib.h>

inherit Integrator;

static void create(Function f) {
    ::create(f);
}

private float *halves(float a, float b) {
    float m;
    m = 0.5 * (a + b);
    return ({
        m,
        0.5 * (b - a),
        0.5 * (a + m),
        0.5 * (m + b),
    });
}

private float simpson(float a, float b, float c, float d) {
    return (a / 6.) * (b + 4. * c + d);
}

private float *leftRightDelta(float h, float fa, float flm, float fm, float frm, float fb, float whole) {
    float left, right, delta;

    left  = simpson(h, fa, flm, fm);
    right = simpson(h, fm, frm, fb);
    delta = left + right - whole;

    return ({ left, right, delta });
}

private float *innerEvals(float lm, float rm) {
    return ({ f->evaluate(lm), f->evaluate(rm) });
}

private float *initialEvals(float a, float b) {
    return ({ f->evaluate(a), f->evaluate(b), f->evaluate(0.5 * (a + b)) });
}

private float integration(float a, float b, float epsilon,
                          float whole, float fa, float fb, float fm, int rectangles) {
    float m, h, lm, rm;
    float flm, frm, left, right, delta;

    ({ m, h, lm, rm }) = halves(a, b);
    ({ flm, frm }) = innerEvals(lm, rm);
    ({ left, right, delta }) = leftRightDelta(h, fa, flm, fm, frm, fb, whole);

    if (rectangles <= 0 || fabs(delta) <= 15. * epsilon) {
        return left + right + (delta) / 15.;
    }

    epsilon *= 0.5;
    --rectangles;

    return integration(a, m, epsilon, left,  fa, fm, flm, rectangles) +
           integration(m, b, epsilon, right, fm, fb, frm, rectangles);
}

private float _integrate(float a, float b) {
    float h, fa, fb, fm, epsilon;

    h = b - a;
    if (h == 0.) {
        return 0.;
    }

    ({ fa, fb, fm }) = initialEvals(a, b);
    epsilon = tolerance;

    return integration(a, b, epsilon, simpson(h, fa, fm, fb), fa, fb, fm, 1000);
}

float integrate(float a, float b, varargs float tolerance) {
    setTolerance(tolerance);
    return _integrate(a, b);
}