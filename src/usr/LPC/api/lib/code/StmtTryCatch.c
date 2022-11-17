# include "Code.h"

inherit LPCStatement;


private LPCStmtBlock caught;	/* try block */
private string exception;	/* exception variable */
private LPCStatement onError;	/* code executed on error */

/*
 * initialize try-catch statement
 */
static void create(LPCStmtBlock stmt1, string str, LPCStatement stmt2,
		   varargs int line)
{
    ::create(line);
    caught = stmt1;
    exception = str;
    onError = stmt2;
}

/*
 * emit code for try-catch statement
 */
void code()
{
    emit("try");
    caught->code();
    emit("catch(" + exception + ")");
    if (onError) {
	onError->code();
    } else {
	emit("{}");
    }
}
