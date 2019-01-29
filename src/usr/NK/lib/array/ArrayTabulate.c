#include <NKlib.h>

inherit Function;
inherit NK_KFUN;

private LongestString longestString;

private string _evaluate(mixed *items, int longestString, int columnsPerRow) {
    int cols;
    string output;
    Iterator iterator;

    iterator = new IntIterator(0, sizeof(items) - 1);
    cols = 0;
    output = "";

    while (!iterator->end()) {
        output += lalign(items[iterator->next()], longestString);
        if (cols == columnsPerRow - 1) {
            output += "\n";
            cols = 0;
        } else {
            cols++;
        }
    }

    return output;
}

string evaluate(mixed *items) {
    int ls, cpr;

    ls = longestString->evaluate(items) + 2;
    cpr = 80 / ls;

    return _evaluate(items, ls, cpr);
}

static void create(void) {
    longestString = new LongestString();
    ::create();
}
