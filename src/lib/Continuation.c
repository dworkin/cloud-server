# include <type.h>
# include <Continuation.h>

# define SYSTEM_AUTO	"/usr/System/lib/auto"


private mixed *continued;	/* list of continuations */
private int started;		/* already started? */

/*
 * initialize a continuation
 */
static void createContinuation(mixed objs, mixed delay, string func,
			       mixed *args)
{
    object origin;
    int clone;

    if (!func) {
	error("Missing function for continuation");
    }

    origin = previous_object();
    sscanf(object_name(origin), "%*s#%d", clone);
    if (clone < 0) {
	error("Continuation in non-persistent object");
    }

    continued = ({ objs, delay, origin, func, args, nil });
}

/*
 * create a standard continuation
 */
static void create(varargs string func, mixed args...)
{
    if (func) {
	createContinuation(FALSE, 0, func, args);
    } else {
	continued = ({ });
    }
}

/*
 * save suspended continuation
 */
void saveContinuation(mixed *continued)
{
    if (previous_program() == SYSTEM_AUTO) {
	::continued = continued;
    }
}

/*
 * add a continuation to the current one
 */
private void addCont(Continuation cont)
{
    mixed *continuation;
    int i;

    if (started || cont->started()) {
	error("Continuation already started");
    }

    continuation = cont->continued();
    if ((i=sizeof(continued)) != 0 && sizeof(continuation) != 0 &&
	continuation[CONT_DELAY] != 0 &&
	continuation[CONT_OBJS] == TRUE &&
	continuation[CONT_FUNC] == "_F_return" &&
	continued[i+=CONT_DELAY - CONT_SIZE] == 0) {
	/*
	 * merge delayed continuation with preceding
	 */
	continued[i] = continuation[CONT_DELAY];
	continuation = continuation[CONT_SIZE ..];
    }
    continued += continuation;
}

/*
 * add continuation or function to current one
 */
object add(mixed func, mixed args...)
{
    object origin;
    int clone;

    if (sizeof(args) == 0 && typeof(func) == T_OBJECT) {
	addCont(func);
    } else {
	if (started) {
	    error("Continuation already started");
	}

	origin = previous_object();
	sscanf(object_name(origin), "%*s#%d", clone);
	if (clone < 0) {
	    error("Continuation in non-persistent object");
	}
	if (typeof(func) != T_STRING) {
	    error("Not a function");
	}
	continued += ({ FALSE, 0, origin, func, args, nil });
    }

    return this_object();
}

/*
 * chain continuation or function to current one
 */
object chain(mixed func, mixed args...)
{
    mixed *continuation;
    object origin;
    int size, clone;

    if (sizeof(args) == 0 && typeof(func) == T_OBJECT) {
	if (sizeof(continuation=func->continued()) == 0) {
	    error("Cannot chain empty continuation");
	}
	switch (typeof(continuation[CONT_OBJS])) {
	case T_OBJECT:
	    error("Cannot chain iterative continuation");

	case T_ARRAY:
	    error("Cannot chain distributed continuation");
	}

	size = sizeof(continued);
	if (size == 0) {
	    error("Cannot chain to empty continuation");
	}
	addCont(func);
	continued[size + CONT_OBJS] = TRUE;
    } else {
	if (started) {
	    error("Continuation already started");
	}

	origin = previous_object();
	sscanf(object_name(origin), "%*s#%d", clone);
	if (clone < 0) {
	    error("Continuation in non-persistent object");
	}
	if (typeof(func) != T_STRING) {
	    error("Not a function");
	}
	continued += ({ TRUE, 0, origin, func, args, nil });
    }

    return this_object();
}

/*
 * operator version of add
 */
static Continuation operator+ (Continuation cont)
{
    return copy_object()->add(cont);
}

/*
 * operator version of chain
 */
static Continuation operator>> (Continuation cont)
{
    return copy_object()->chain(cont);
}

/*
 * start the continuation
 */
atomic void runNext(mixed args...)
{
    if (started) {
	error("Continuation already started");
    }
    if (sizeof(continued) != 0) {
	if (!continued[CONT_ORIGIN]) {
	    error("No environment for continuation");
	}

	continued[CONT_ARGS] += args;
	::startContinuation(continued, FALSE);
    }
    started = TRUE;
}

/*
 * start the continuation concurrently
 */
atomic void runParallel(mixed args...)
{
    if (started) {
	error("Continuation already started");
    }
    if (sizeof(continued) != 0) {
	if (!continued[CONT_ORIGIN]) {
	    error("No environment for continuation");
	}

	continued[CONT_ARGS] += args;
	::startContinuation(continued, TRUE);
    }
    started = TRUE;
}


mixed *continued()
{
    if (previous_program() == OBJECT_PATH(Continuation)) {
	return continued;
    }
}

int started()	{ return started; }
