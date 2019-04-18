# include <Continuation.h>

# define SYSTEM_AUTO	"/usr/System/lib/auto"


private mixed *continued;	/* saved continuation */

/*
 * save suspended continuation in this token
 */
void saveContinuation(mixed *continued)
{
    if (previous_program() == SYSTEM_AUTO) {
	::continued = continued;
    }
}

/*
 * resume continuation saved in this token
 */
void resumeContinuation(varargs mixed arg)
{
    if (!continued) {
	error("No continuation");
    }
    if (!continued[CONT_ORIGIN]) {
	error("No environment for Continuation");
    }
    continued[CONT_ORIGIN]->_F_resume(continued, arg);
    continued = nil;
}
