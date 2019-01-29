#include <NKlib.h>

inherit Polynomial;

static void create(int numberOfDice, int numberOfFaces) {
    Polynomial p;
    float *base;
    int i;

    base = allocate_float(numberOfFaces + 1);
    for (i = 1; i <= numberOfFaces; i++) {
        base[i] = 1.;
    }

    p = new Polynomial(base);

    if (numberOfDice > 1) {
        for (i = 1; i < numberOfDice; i++) {
            p *= new Polynomial(base);
        }
    }

    ::create(p->coefficients());
}
