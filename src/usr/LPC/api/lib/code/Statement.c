# include "Code.h"

inherit LPCCode;


private LPCStatement next;	/* next in list */

/*
 * set the next statement in the list
 */
void setNext(LPCStatement stmt)
{
    next = stmt;
}

/*
 * return the next statement in the list
 */
LPCStatement next()
{
    return next;
}

/*
 * emit code for empty statement
 */
void code()
{
    emit(";", line());
}
