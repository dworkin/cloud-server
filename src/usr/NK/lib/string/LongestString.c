#include <NKlib.h>

inherit Function;

private mixed *items;
private Stringify stringify;

int evaluate(mixed *args) {
    Iterator iterator;
    int x, y;

    x = 0;
    y = 0;
    iterator = new IntIterator(0, sizeof(args) - 1);
    while (!iterator->end()) {
        x = strlen(stringify->evaluate(args[iterator->next()]));
        if (x > y) {
            y = x;
        }
    }

    return y;
}

void apply(mixed *args) {
    items = args;
}

static void create(void) {
    stringify = new Stringify();
    ::create();
}
