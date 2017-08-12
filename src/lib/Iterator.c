# include <Iterator.h>

private Iterable iterated;	/* object iterated over */
private mixed state;		/* iteration state */
private mixed item;		/* current iterated element */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize Iterator
 */
static void create(Iterable obj, mixed newState)
{
    iterated = obj;
    state = newState;
    item = nil;
}

/*
 * NAME:	reset()
 * DESCRIPTION:	reset Iterator to first element
 */
void reset()
{
    state = iterated->iteratorStart(state);
    item = nil;
}

/*
 * NAME:	current()
 * DESCRIPTION:	get current item from Iterator
 */
mixed current()
{
    return item;
}

/*
 * NAME:	next()
 * DESCRIPTION:	get next item from Iterator
 */
mixed next()
{
    ({ state, item }) = iterated->iteratorNext(state);
    return item;
}

/*
 * NAME:	end()
 * DESCRIPTION:	return TRUE if the end has been reached
 */
int end()
{
    return iterated->iteratorEnd(state);
}


Iterable iterated()	{ return iterated; }
