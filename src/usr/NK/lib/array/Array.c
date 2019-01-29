#include <NKlib.h>

inherit Iterable;
inherit CheckEquals;
inherit Sort;
inherit NK_KFUN;

mixed *array;
private int iterationCount;

int size(void) {
    return sizeof(array);
}

mixed get(int index) {
    if (index < 0 || index >= size()) {
        error("Array: index " + index + " is out of range.");
    }

    return array[index];
}

int equals(Array that) {
    Iterator iterator;
    int i;
    mixed thisValue, thatValue;

    if (size() != that->size()) {
        return FALSE;
    }

    iterator = new IntIterator(0, size() - 1);
    while (!iterator->end()) {
        i = iterator->next();
        thisValue = get(i);
        thatValue = array->get(i);
        if (!checkEquals(thisValue, thatValue)) {
            return FALSE;
        }
    }

    return TRUE;
}

int contains(mixed value) {
    Iterator iterator;
    mixed x;
    int s;
    string err;

    iterator = new Iterator(this_object(), 0, size() - 1);
    while (!iterator->end()) {
        x = iterator->next();

        if (T_OBJECT == typeof(x) && function_object("equals", (mixed) x)) {
            err = catch(s = x->equals(value));
            if (!err && s) {
                return TRUE;
            }
        } else {
            if (x == value) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

mixed *copy(void) {
    mixed *copy;
    Iterator iterator;
    int i;

    if (!size()) {
        return ({ });
    }

    copy = allocate(size());
    iterator = new IntIterator(0, size() - 1);
    while (!iterator->end()) {
        i = iterator->next();
        copy[i] = get(i);
    }

    return copy;
}

mixed *toArray(void) {
    return array;
}

void sort(void) {
    if (sizeof(array) < 2) {
        return;
    }

    ::sort(array);
}

private int isLastIteration(int from, int to) {
    return iterationCount == to - from + 1;
}

static mixed operator[] (int index) {
    return get(index);
}

static Array operator[..] (mixed from, mixed to) {
    if (from == nil) {
        from = 0;
    }
    if (to == nil) {
        to = size() - 1;
    }
    return new Array(array[from..to]);
}

static void operator[]= (int index, mixed value) {
    array[index] = value;
}

Array union(Array x) {
    return new Array(toArray() | x->toArray());
}

Array intersection(Array x) {
    return new Array(toArray() & x->toArray());
}

Array symmetricDifference(Array x) {
    return new Array(toArray() ^ x->toArray());
}

Array difference(Array x) {
    return intersection(symmetricDifference(x));
}

static Array operator| (Array x) {
    return this_object()->union(x);
}

static Array operator& (Array x) {
    return this_object()->intersection(x);
}

static Array operator^ (Array x) {
    return this_object()->symmetricDifference(x);
}

static Array operator- (Array x) {
    return this_object()->difference(x);
}

private int *fixFromTo(int from, int to) {
    if (!from) {
        from = 0;
    }

    if (!to) {
        to = size() - 1;
    }
    return ({ from, to });
}

void each(Function eachFunction, varargs int from, int to) {
    Iterator iterator;
    int i;
    mixed m;

    ({ from, to }) = fixFromTo(from, to);
    iterator = new IntIterator(from, to);

    while (!iterator->end()) {
        i = iterator->next();
        m = get(i);
        eachFunction->evaluate(m, i);
    }
}

Array filter(Function filterer, varargs int from, int to) {
    Iterator iterator;
    Array filteredArray;
    mixed element;
    int i, fCount;

    ({ from, to }) = fixFromTo(from, to);
    iterator = new Iterator(this_object(), from, to);

    while (!iterator->end()) {
        element = iterator->next();
        if (filterer->evaluate(element)) {
            fCount ++;
        }
    }

    filteredArray = new Array(allocate(fCount));
    iterator->reset();
    i = 0;

    while (!iterator->end()) {
        element = iterator->next();
        if (filterer->evaluate(element)) {
            filteredArray[i++] = element;
        }
    }

    return filteredArray;
}

Array map(Function mapper, varargs int from, int to) {
    Iterator iterator;
    Array mappedArray;
    mixed element;
    int i;

    ({ from, to }) = fixFromTo(from, to);
    iterator = new Iterator(this_object(), from, to);
    mappedArray = new Array(allocate(to - from + 1));
    i = 0;

    while (!iterator->end()) {
        element = iterator->next();
        mappedArray[i] = mapper->evaluate(element);
        i++;
    }
    return mappedArray;
}

mixed reduce(Function reducer, varargs int from, int to, int evalFirstVal) {
    Iterator iterator;
    mixed nextValue, currentValue;

    ({ from, to }) = fixFromTo(from, to);
    iterator = new Iterator(this_object(), from, to);
    currentValue = iterator->next();
    if (evalFirstVal) {
        currentValue = reducer->evaluate(nil, currentValue, isLastIteration(from, to));
    }

    while (!iterator->end()) {
        nextValue = iterator->next();
        currentValue = reducer->evaluate(nextValue, currentValue, isLastIteration(from, to));
    }

    return currentValue;
}

string tabulate(void) {
    Function tabulator;

    tabulator = new ArrayTabulate();

    return tabulator->evaluate(toArray());
}

string box(Function reducer) {
    return reducer->evaluate(toArray());
}

Array unique(void) {
    Iterator iterator;
    mixed *u;

    if (!size()) {
        return new Array(({ }));
    }

    if (size() == 1) {
        if (get(0) == nil) {
            return new Array(({ }));
        }
        return new Array(toArray());
    }

    u = allocate(size());
    iterator = new IntIterator(0, size() - 1);
    while (!iterator->end()) {
        u -= ({ get(iterator->next()) });
        u += ({ get(iterator->current()) });
    }

    u -= ({ nil });

    return new Array(u);
}

mixed iteratorStart(mixed from, mixed to) {
    return ({ from, to, from, 0 });
}

mixed *iteratorNext(mixed state) {
    int from, to, index;
    mixed value;

    ({ from, to, index, iterationCount }) = state;
    value = array[index];
    index++;
    iterationCount++;

    return ({ ({ from, to, index, iterationCount }), value });
}

int iteratorEnd(mixed state) {
    int from, to, index;

    ({ from, to, index, iterationCount }) = state;

    return isLastIteration(from, to);
}

static void create(mixed *array) {
    ::array = array;
}
