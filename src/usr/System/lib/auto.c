# include <kernel/kernel.h>
# include <type.h>
# include "objectd.h"


/*
 * NAME:	_F_init()
 * DESCRIPTION:	initialization call gate
 */
nomask void _F_init(mixed *args)
{
    if (previous_program() == "/usr/System/lib/auto") {
	this_object()->init(args...);
    }
}

/*
 * NAME:	clone_object()
 * DESCRIPTION:	create and initialize a new clone
 */
static object clone_object(string path, mixed args...)
{
    object clone;

    clone = ::clone_object(path);
    clone->_F_init(args);
    return clone;
}

/*
 * NAME:	new_object()
 * DESCRIPTION:	create and initialize a new light-weight object
 */
static object new_object(string path, mixed args...)
{
    object new;

    new = ::new_object(path);
    new->_F_init(args);
    return new;
}

/*
 * NAME:	copy_object()
 * DESCRIPTION:	copy a light-weight object
 */
static object copy_object(object obj)
{
    if (!obj || sscanf(object_name(obj), "%*s#-1") == 0) {
	error("Bad argument 1 for function copy_object");
    }
    return ::new_object(obj);
}

/*
 * NAME:	_F_touch()
 * DESCRIPTION:	touch call gate
 */
nomask int _F_touch(string func)
{
    if (previous_program() == OBJECTD) {
	if (!function_object("patch", this_object())) {
	    /* patch function doesn't exist yet */
	    return TRUE;
	}

	this_object()->patch();
    }
    return FALSE;
}


/*
 * Disabled functions.
 */
static void save_object(string file)		{ return; }
static string query_ip_number(object obj)	{ return nil; }
static string query_ip_name(object obj)		{ return nil; }
