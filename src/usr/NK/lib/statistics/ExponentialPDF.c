#include <NKlib.h>

inherit Function;

private float m;

private void assertParameter(float m) {
    if (m == 0.) {
        error("ExponentialPDF: division by zero.");
    }
}

void apply(float m) {
    assertParameter(m);
    ::m = 1. / m;
}

float evaluate(float x) {
    return m * exp(-m * x);
}

static void create(float m) {
    ::create();
    apply(m);
}
