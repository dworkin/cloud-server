# include <Iterator.h>

private Iterable iterated;	/* object iterated over */
private mixed from, to;		/* iteration range */
private mixed state;		/* iteration state */
private mixed item;		/* current iterated element */

/*
 * reset Iterator to first element
 */
void reset(void)
{
    state = iterated->iteratorStart(from, to);
    item = nil;
}

/*
 * initialize Iterator
 */
static void create(Iterable obj, mixed from, mixed to)
{
    iterated = obj;
    ::from = from;
    ::to = to;
    reset();
}

/*
 * get current item from Iterator
 */
mixed current(void)
{
    return item;
}

/*
 * get next item from Iterator
 */
mixed next(void)
{
    ({ state, item }) = iterated->iteratorNext(state);
    return item;
}

/*
 * return TRUE if the end has been reached
 */
int end(void)
{
    return iterated->iteratorEnd(state);
}


Iterable iterated()	{ return iterated; }
