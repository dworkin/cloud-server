# include <Iterator.h>

private Iterable iterated;	/* object iterated over */
private mixed state;		/* iteration state */
private mixed item;		/* current iterated element */

/*
 * initialize Iterator
 */
static void create(Iterable obj, mixed newState)
{
    iterated = obj;
    state = newState;
    item = nil;
}

/*
 * reset Iterator to first element
 */
void reset()
{
    state = iterated->iteratorStart(state);
    item = nil;
}

/*
 * get current item from Iterator
 */
mixed current()
{
    return item;
}

/*
 * get next item from Iterator
 */
mixed next()
{
    ({ state, item }) = iterated->iteratorNext(state);
    return item;
}

/*
 * return TRUE if the end has been reached
 */
int end()
{
    return iterated->iteratorEnd(state);
}


Iterable iterated()	{ return iterated; }
