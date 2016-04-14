# include <Iterator.h>

private Iterable iterated;	/* object iterated over */
private mixed state;		/* iteration state */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize Iterator
 */
static void create(Iterable obj, mixed newState)
{
    iterated = obj;
    state = newState;
}

/*
 * NAME:	reset()
 * DESCRIPTION:	reset Iterator to first element
 */
void reset()
{
    state = iterated->iteratorStart(state);
}

/*
 * NAME:	next()
 * DESCRIPTION:	get next item from Iterator
 */
mixed next()
{
    mixed result;

    ({ state, result }) = iterated->iteratorNext(state);
    return result;
}

/*
 * NAME:	end()
 * DESCRIPTION:	return TRUE if the end has been reached
 */
int end()
{
    return iterated->iteratorEnd(state);
}
