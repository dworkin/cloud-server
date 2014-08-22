# include "code.h"

inherit Code;


private string declaration;	/* int* n */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize declaration
 */
static void create(string type, string name, varargs int line)
{
    ::create(line);
    declaration = type + " " + name;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for declaration
 */
void code()
{
    emit(declaration, line());
}
