#include <NKlib.h>

inherit CheckEquals;
inherit Sort;

private mixed *array;

mixed *binary(mixed target) {
    int a, b, c, d;
    mixed e;
    
    a = 0;
    b = sizeof(array) - 1;
    c = -1;
    d = 0;

    while (a <= b) {
        d++;
        c = a + (b - a) / 2;

        if (checkEquals(array[c], target)) {
            e = c;
            break;
        }

        if (array[c] < target) {
            a = c + 1;
        } else {
            b = c - 1;
        }
    }

    return ({ e, d });
}

private void init(int shouldSort) {
    if (shouldSort) {
        sort(array);
    }
}

static void create(mixed *array, varargs int shouldSort) {
    ::array = array;
    init(shouldSort);
}