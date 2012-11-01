# include <kernel/kernel.h>
# include <type.h>


/*
 * NAME:	_F_patch()
 * DESCRIPTION:	upgrade the dataspace of this object by patching it
 */
nomask void _F_patch()
{
    if (previous_program() == "/usr/System/obj/wiztool") {
	this_object()->patch(sscanf(object_name(this_object()),
				    "%*s/obj/%*s#") != 1);
    }
}

/*
 * NAME:	new_object()
 * DESCRIPTION:	create and initialize a new light-weight object
 */
static object new_object(string path, mixed args...)
{
    object new;

    new = ::new_object(path);
    new->_F_data(args);
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
 * Disabled functions.
 */
static void save_object(string file)		{ return; }
static string query_ip_number(object obj)	{ return nil; }
static string query_ip_name(object obj)		{ return nil; }
