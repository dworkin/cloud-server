#include <NKlib.h>

inherit Function;

float evaluate(float covariance, Statistics a, Statistics b) {
    float af, bf;

    af = a->getStandardDeviation();
    bf = b->getStandardDeviation();

    if (af == 0. || bf == 0.) {
        error("Correlation: zero standard deviation: " + af + ", " + bf);
    }

    return covariance / a->getStandardDeviation() / b->getStandardDeviation();
}
