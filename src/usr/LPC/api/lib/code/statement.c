# include "code.h"

inherit LPCCode;


private LPCStatement next;	/* next in list */

/*
 * NAME:	setNext()
 * DESCRIPTION:	set the next statement in the list
 */
void setNext(LPCStatement stmt)
{
    next = stmt;
}

/*
 * NAME:	next()
 * DESCRIPTION:	return the next statement in the list
 */
LPCStatement next()
{
    return next;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for empty statement
 */
void code()
{
    emit(";", line());
}
