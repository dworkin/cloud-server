# include "code.h"

inherit Code;


private Statement next;		/* next in list */

/*
 * NAME:	setNext()
 * DESCRIPTION:	set the next statement in the list
 */
void setNext(Statement stmt)
{
    next = stmt;
}

/*
 * NAME:	next()
 * DESCRIPTION:	return the next statement in the list
 */
Statement next()
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
