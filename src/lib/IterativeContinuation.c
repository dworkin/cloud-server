# include <Iterator.h>
# include <Continuation.h>

inherit Continuation;


/*
 * create an iterative continuation
 */
static void create(string func, Iterator iter, mixed args...)
{
    if (!iter) {
	error("Missing iterator");
    }
    createContinuation(iter, 0, func, args);
}
