# include <type.h>
# include <Continuation.h>

inherit Continuation;


/*
 * create delayed continuation
 */
static void create(mixed delay)
{
    switch (typeof(delay)) {
    case T_INT:
	if (delay > 0) {
	    break;
	}
	error("Invalid delay");

    case T_FLOAT:
	if (delay > 0.0) {
	    break;
	}
	/* fall through */
    default:
	error("Invalid delay");
    }

    createContinuation(TRUE, delay, "_F_return", ({ }));
}
