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
    if (continuation[CONT_DELAY] != 0 && continuation[CONT_OBJS] == TRUE &&
	continuation[CONT_FUNC] == "_F_return" &&
	continued[i=(sizeof(continued) - CONT_SIZE + CONT_DELAY)] == 0) {
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
void add(mixed func, mixed args...)
{
    object origin;
    int clone;

    if (sizeof(args) == 0 && typeof(func) == T_OBJECT) {
	addCont(func);
	return;
    }
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

/*
 * chain continuation or function to current one
 */
void chain(mixed func, mixed args...)
{
    object origin;
    int size, clone;

    if (sizeof(args) == 0 && typeof(func) == T_OBJECT) {
	switch (typeof(func->continued()[CONT_OBJS])) {
	case T_OBJECT:
	    error("Cannot chain iterative continuation");

	case T_ARRAY:
	    error("Cannot chain distributed continuation");
	}

	size = sizeof(continued);
	addCont(func);
	continued[size + CONT_OBJS] = TRUE;
	return;
    }
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

/*
 * operator version of add
 */
static Continuation operator+ (Continuation cont)
{
    object obj;

    obj = copy_object();
    obj->add(cont);
    return obj;
}

/*
 * operator version of chain
 */
static Continuation operator>> (Continuation cont)
{
    object obj;

    obj = copy_object();
    obj->chain(cont);
    return obj;
}

/*
 * start the continuation
 */
atomic void runNext(varargs mixed arg)
{
    if (started) {
	error("Continuation already started");
    }
    if (!continued[CONT_ORIGIN]) {
	error("No environment for Continuation");
    }

    continued[CONT_VAL] = arg;
    ::startContinuation(continued, FALSE);
    started = TRUE;
}

/*
 * start the continuation concurrently
 */
atomic void runParallel(varargs mixed arg)
{
    if (started) {
	error("Continuation already started");
    }
    if (!continued[CONT_ORIGIN]) {
	error("No environment for Continuation");
    }

    continued[CONT_VAL] = arg;
    ::startContinuation(continued, TRUE);
    started = TRUE;
}


mixed *continued()
{
    if (previous_program() == OBJECT_PATH(Continuation)) {
	return continued;
    }
}

int started()	{ return started; }
