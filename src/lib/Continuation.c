# include <type.h>
# include "Continuation.h"


private mixed *continued;	/* list of continuations */
private object origin;		/* persistent environment */
private int started;		/* already started? */

/*
 * initialize a continuation
 */
static void createContinuation(mixed objs, mixed delay, string func,
			       mixed *args)
{
    int clone;

    if (!func) {
	error("Missing function for continuation");
    }
    origin = previous_object();
    sscanf(object_name(origin), "%*s#%d", clone);
    if (clone < 0) {
	error("Continuation in non-persistent object");
    }

    continued = ({ ({ objs, delay, func, args }) });
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
    if (started || cont->started()) {
	error("Continuation already started");
    }
    if (origin != cont->origin()) {
	error("Continuations in different objects");
    }

    continued += cont->continued();
}

/*
 * chain a continuation to the current one
 */
private void chainCont(Continuation cont)
{
    mixed *continuation;
    int size;

    if (started || cont->started()) {
	error("Continuation already started");
    }
    if (origin != cont->origin()) {
	error("Continuations in different objects");
    }

    continuation = cont->continued();
    switch (typeof(continuation[0][0])) {
    case T_OBJECT:
	error("Cannot chain iterative continuation");

    case T_ARRAY:
	error("Cannot chain external continuation");
    }

    size = sizeof(continued);
    continued += continuation;
    continued[size][0] = TRUE;
}

/*
 * add continuation or function to current one
 */
void add(mixed func, mixed args...)
{
    if (sizeof(args) == 0 && typeof(func) == T_OBJECT) {
	addCont(func);
	return;
    }
    if (started) {
	error("Continuation already started");
    }
    if (typeof(func) != T_STRING) {
	error("Not a function");
    }
    continued += ({ ({ FALSE, 0, func, args }) });
}

/*
 * chain continuation or function to current one
 */
void chain(mixed func, mixed args...)
{
    if (sizeof(args) == 0 && typeof(func) == T_OBJECT) {
	chainCont(func);
	return;
    }
    if (started) {
	error("Continuation already started");
    }
    if (typeof(func) != T_STRING) {
	error("Not a function");
    }
    continued += ({ ({ TRUE, 0, func, args }) });
}

/*
 * operator version of add
 */
static atomic Continuation operator+ (Continuation cont)
{
    mixed *old;
    object obj;

    old = continued;
    addCont(cont);
    obj = copy_object();
    continued = old;
    return obj;
}

/*
 * operator version of chain
 */
static atomic Continuation operator>> (Continuation cont)
{
    mixed *old;
    object obj;

    old = continued;
    chainCont(cont);
    obj = copy_object();
    continued = old;
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
    if (!origin) {
	error("No environment for Continuation");
    }

    ::startContinuation(origin, continued, FALSE);
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
    if (!origin) {
	error("No environment for Continuation");
    }

    ::startContinuation(origin, continued, TRUE);
    started = TRUE;
}


mixed *continued()
{
    if (previous_program() == OBJECT_PATH(Continuation)) {
	return continued;
    }
}

object origin()	{ return origin; }
int started()	{ return started; }
