# include "code.h"

inherit LPCStatement;


private LPCStmtBlock caught;	/* block inside catch */
private LPCStatement onError;	/* code executed on error */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize catch statement
 */
static void create(LPCStmtBlock stmt1, LPCStatement stmt2, varargs int line)
{
    ::create(line);
    caught = stmt1;
    onError = stmt2;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for catch statement
 */
void code()
{
    emit("catch");
    caught->code();
    if (onError) {
	emit(":");
	onError->code();
    }
}
