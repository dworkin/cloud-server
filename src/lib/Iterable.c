# include <Iterator.h>

mixed iteratorStart(mixed state);
mixed *iteratorNext(mixed state);
int iteratorEnd(mixed state);

/*
 * default implementation of iterator creation
 */
Iterator iterator()
{
    return new Iterator(this_object(), iteratorStart(nil));
}
