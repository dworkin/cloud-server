# include <type.h>
# include "Continuation.h"

inherit Continuation;


/*
 * create a distributed continuation
 */
static void create(object *objs, mixed timeout, string func, mixed args...)
{
    if (!objs) {
	error("Missing list of objects");
    }
    switch (typeof(timeout)) {
    case T_INT:
	if (timeout <= 0) {
	    error("Invalid timeout");
	}
	break;

    case T_FLOAT:
	if (timeout <= 0.0) {
	    error("Invalid timeout");
	}
	break;

    default:
	error("Invalid timeout");
    }

    createContinuation(objs, timeout, func, args);
}
