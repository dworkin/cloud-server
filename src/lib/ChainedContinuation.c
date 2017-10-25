# include "Continuation.h"

inherit Continuation;


/*
 * create continuation that uses preceding returned value
 */
static void create(string func, mixed args...)
{
    createContinuation(TRUE, 0, func, args);
}
