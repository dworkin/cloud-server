# include <Iterator.h>

mixed iteratorStart(mixed from, mixed to);
mixed *iteratorNext(mixed state);
int iteratorEnd(mixed state);

/*
 * default implementation of iterator creation
 */
Iterator iterator(varargs mixed from, mixed to)
{
    return new Iterator(this_object(), from, to);
}
