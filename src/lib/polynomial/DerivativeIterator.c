#include <NKlib.h>

inherit Iterable;
inherit Iterator;

private Polynomial p;

mixed iteratorStart(mixed from, mixed to) {
    return ({ from });
}

mixed *iteratorNext(mixed state) {
    int i;
    float f;

    ({ i }) = state;

    f = p[i];
    i--;

    return ({ ({ i }), f });
}

int iteratorEnd(mixed state) {
    int i;

    ({ i }) = state;

    return i < 1;
}

static void create(Polynomial p) {
    ::p = p;
    ::create(this_object(), p->degree(), 0);
}
