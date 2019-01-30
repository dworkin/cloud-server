#include <NKlib.h>

static Function f;
static float tolerance;

void setFunction(Function f) {
    ::f = f;
}

void setTolerance(float tolerance) {
    if (!tolerance) {
        tolerance = 1e-7;
    }
    ::tolerance = tolerance;
}

float integrate(float lowerLimit, float upperLimit, varargs float tolerance) {
    return 0.;
}

static void create(Function f) {
    setFunction(f);
}
