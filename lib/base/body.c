# define Soul		"/lib/base/soul"


private object soul;	/* controller of this body */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize body
 */
static void create()
{
}

/*
 * NAME:	_F_set_soul()
 * DESCRIPTION:	set controller of this body
 */
nomask void _F_set_soul(object obj)
{
    if (previous_program() != Soul || soul) {
	error("bad soul");
    }
    soul = obj;
}

/*
 * NAME:	_F_remove_soul()
 * DESCRIPTION:	remove controller of this body
 */
nomask void _F_remove_soul(object obj)
{
    if (previous_program() != Soul || soul != obj) {
	error("bad soul");
    }
    soul = nil;
}

/*
 * NAME:	query_soul()
 * DESCRIPTION:	return the current soul
 */
object query_soul()
{
    return soul;
}

/*
 * NAME:	_F_command()
 * DESCRIPTION:	process soul-supplied command
 */
nomask mixed _F_command(string command, mixed *args)
{
    if (previous_program() == Soul && previous_object() == soul) {
	return call_limited("command", command, args);
    }
}
