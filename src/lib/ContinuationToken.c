# include <Continuation.h>

# define SYSTEM_AUTO	"/usr/System/lib/auto"


private mixed *continued;	/* saved continuation */
private object origin;		/* origin of continuation */

/*
 * save suspended continuation in this token
 */
void saveContinuation(mixed *continued, object origin)
{
    if (previous_program() == SYSTEM_AUTO) {
	::continued = continued;
	::origin = origin;
    }
}

/*
 * wake up continuation saved in this token
 */
void wakeContinuation(varargs mixed arg)
{
    if (!continued) {
	error("No continuation");
    }
    origin->_F_wake(continued, arg);
    continued = nil;
}
