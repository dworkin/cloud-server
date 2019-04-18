# include <type.h>
# include <Continuation.h>


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

    continued = ({ ({ objs, delay, origin, func, args }) });
}

/*
 * create a standard continuation
 */
static void create(string func, mixed args...)
{
    createContinuation(FALSE, 0, func, args);
}

/*
 * add a continuation to the current one
 */
private void addCont(Continuation cont)
{
    mixed *continuation, *params;
    int i;

    if (started || cont->started()) {
	error("Continuation already started");
    }

    continuation = cont->continued();
    params = continuation[0];
    if (params[CONT_DELAY] != 0 && params[CONT_OBJS] == TRUE &&
	params[CONT_FUNC] == "_F_return" &&
	continued[i=sizeof(continued) - 1][CONT_DELAY] == 0) {
	/*
	 * merge delayed continuation with preceding
	 */
	continued[i][CONT_DELAY] = params[CONT_DELAY];
	continuation = continuation[1 ..];
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
    continued += ({ ({ FALSE, 0, origin, func, args }) });
}

/*
 * chain continuation or function to current one
 */
void chain(mixed func, mixed args...)
{
    object origin;
    int size, clone;

    if (sizeof(args) == 0 && typeof(func) == T_OBJECT) {
	switch (typeof(func->continued()[0][CONT_OBJS])) {
	case T_OBJECT:
	    error("Cannot chain iterative continuation");

	case T_ARRAY:
	    error("Cannot chain distributed continuation");
	}

	size = sizeof(continued);
	addCont(func);
	continued[size][CONT_OBJS] = TRUE;
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
    continued += ({ ({ TRUE, 0, origin, func, args }) });
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
atomic void runNext()
{
    if (started) {
	error("Continuation already started");
    }
    if (!continued[0][CONT_ORIGIN]) {
	error("No environment for Continuation");
    }

    ::startContinuation(continued, FALSE);
    started = TRUE;
}

/*
 * start the continuation concurrently
 */
atomic void runParallel()
{
    if (started) {
	error("Continuation already started");
    }
    if (!continued[0][CONT_ORIGIN]) {
	error("No environment for Continuation");
    }

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
