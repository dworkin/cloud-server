#include <NKlib.h>

inherit Function;

float evaluate(Statistics a, Statistics b) {
    Iterator iterator;
    float sum;
    int i;

    if (a->size() != b->size()) {
        error("Covariance: the sample sizes do not match.");
    }

    if (a->size() < 2) {
        error("Covariance: the sample size must be greater than unity.");
    }

    sum = 0.;
    iterator = new IntIterator(0, a->size() - 1);
    while (!iterator->end()) {
        i = iterator->next();
        sum += (a->get(i) - a->getMean()) * (b->get(i) - b->getMean());
    }

    return sum / (float) (a->size() - 1);
}
