#include <NKlib.h>

inherit Function;
inherit NK_KFUN;

private Polynomial poly;
private int n;
private int r;
private int s;

void apply(int s) {
    ::s = s;
}

mapping evaluate(void) {
    Iterator iterator;
    string *polys, *pq;
    int i, sz;
    int a, b, min_b, smb;
    float nr, p;
    Array probabilities;
    Function reducer;
    mapping returnValue;

    returnValue = ([ ]);
    nr = pow((float) n, (float) r);
    polys = explode(poly->toString(), " + ");
    i = 0;
    sz = sizeof(polys);
    probabilities = new Array(allocate(sz));
    reducer = new ArrayArithmeticReducer(ARRAY_ARITHMETIC_ADD);
    iterator = new IntIterator(i, sz - 1);

    while (sizeof(polys) && !iterator->end()) {
        pq = explode(polys[iterator->next()], "x^");

        if (sizeof(pq) == 1) {
            a = 1;
            sscanf(pq[0], "%d", b);
        } else {
            sscanf(pq[0], "%d", a);
            sscanf(pq[1], "%d", b);
        }

        if (iterator->current() == 0) {
            min_b = b;
        }

        p = (float) a / nr;
        probabilities[iterator->current()] = p;
    }

    smb = s - min_b;

    if (smb == 0) {
        returnValue["<"] = 0.;
        returnValue["="] = probabilities[0];
        returnValue[">"] = 1.;
    } else if (smb == sz - 1) {
        returnValue["<"] = 1.;
        returnValue["="] = probabilities[sz - 1];
        returnValue[">"] = 0.;
    } else if (smb < 1) {
        returnValue["<"] = 0.;
        returnValue["="] = 0.;
        returnValue[">"] = 1.;
    } else if (smb > sz) {
        returnValue["<"] = 1.;
        returnValue["="] = 0.;
        returnValue[">"] = 0.;
    } else {
        returnValue["<"] = probabilities[.. smb - 1]->reduce(reducer);
        returnValue["="] = probabilities[smb];
        returnValue[">"] = probabilities[smb + 1 ..]->reduce(reducer);
    }

    return returnValue;
}

static void create(Polynomial poly, int n, int r) {
    ::create();
    ::poly = poly;
    ::n = n;
    ::r = r;
}
