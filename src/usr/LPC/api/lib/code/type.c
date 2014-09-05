# include "code.h"

inherit LPCCode;


private string classType;	/* class + type */
private LPCExpression objType;	/* object type */
private int indirection;	/* *** */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize declaration
 */
static void create(string typeStr, LPCExpression typeName, int ind,
		   varargs int line)
{
    ::create(line);
    classType = typeStr;
    objType = typeName;
    indirection = ind;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for declaration
 */
void code()
{
    emit(classType + " ", line());
    if (objType) {
	objType->code();
	emit(" ");
    }
    if (indirection) {
	emit("***************"[.. indirection - 1]);
    }
}
