# include "code.h"

inherit Statement;


private StmtBlock caught;	/* block inside catch */
private Statement onError;	/* code executed on error */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize catch statement
 */
static void create(StmtBlock stmt1, Statement stmt2, varargs int line)
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
