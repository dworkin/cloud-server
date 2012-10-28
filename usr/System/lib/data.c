/*
 * This object is inherited by all light-weight objects.
 */

inherit "auto";


/*
 * NAME:	_F_data()
 * DESCRIPTION:	initializer call gate
 */
nomask void _F_data(mixed *args)
{
    if (previous_program() == "/usr/System/lib/auto") {
	this_object()->init(args...);
    }
}
