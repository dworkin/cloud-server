# include <Iterator.h>

inherit Iterable;
inherit Iterator;


private int step;	/* +1 or -1 */

/*
 * return the initial state
 */
mixed iteratorStart(mixed from, mixed to)
{
    return ({ from, to });
}

/*
 * next iteration
 */
mixed *iteratorNext(mixed state)
{
    int from, to;

    ({ from, to }) = state;
    if (from - step == to) {
	return ({ ({ from, to }), nil });
    }

    return ({ ({ from + step, to }), from });
}

/*
 * reached the end?
 */
int iteratorEnd(mixed state)
{
    return (state[0] - step == state[1]);
}

/*
 * create an integer iterator
 */
static void create(int from, int to)
{
    step = (from <= to) ? 1 : -1;
    ::create(this_object(), from, to);
}
