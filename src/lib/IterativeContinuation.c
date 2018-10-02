# include <Iterator.h>
# include <Continuation.h>

inherit Continuation;


/*
 * create an iterative continuation
 */
static void create(Iterator iter, string func, mixed args...)
{
    if (!iter) {
	error("Missing iterator");
    }
    createContinuation(iter, 0, func, args);
}
