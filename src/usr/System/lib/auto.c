# include <kernel/kernel.h>
# include <status.h>

# define ObjectServer	"/usr/System/sys/objectd"
# define UpgradeServer	"/usr/System/sys/upgraded"


/*
 * NAME:	_F_init()
 * DESCRIPTION:	System level creator function
 */
nomask int _F_init()
{
    if (previous_program() == AUTO) {
	mixed *args;

	args = DRIVER->get_tlvar(0);
	if (args) {
	    create(args...);
	} else {
	    create();
	}
    }
    return TRUE;
}

/*
 * NAME:	clone_object()
 * DESCRIPTION:	create and initialize a new clone
 */
static object clone_object(string path, mixed args...)
{
    DRIVER->set_tlvar(0, args);
    return ::clone_object(path);
}

/*
 * NAME:	new_object()
 * DESCRIPTION:	create and initialize a new light-weight object
 */
static object new_object(string path, mixed args...)
{
    object new;

    DRIVER->set_tlvar(0, args);
    return ::new_object(path);
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
nomask int _F_touch()
{
    if (previous_program() == ObjectServer) {
	if (UpgradeServer->query_upgrading()) {
	    return TRUE;	/* still upgrading */
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
