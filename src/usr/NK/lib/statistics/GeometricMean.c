#include <NKlib.h>

inherit Function;

float evaluate(float *sample) {
    float x;
    Iterator iterator;

    if (sizeof(sample) == 0) {
        error("GeometricMean: size of sample must be at least unity.");
    }

    x = 1.;
    iterator = new IntIterator(0, sizeof(sample) - 1);
    while (!iterator->end()) {
        if ((float) sample[iterator->next()] <= 0.) {
            error("GeometricMean: sample of positive different from zero numbers only.");
        }
        x *= (float) sample[iterator->current()];
    }

    return pow(x, 1. / (float) sizeof(sample));
}
