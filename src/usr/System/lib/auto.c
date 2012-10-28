# include <kernel/kernel.h>
# include <type.h>


# define Clone	"/usr/System/lib/clone"

/*
 * NAME:	_F_init()
 * DESCRIPTION:	extra initialisation function
 */
static nomask void _F_init(int clone)
{
    if (previous_program() == AUTO && this_object() <- Clone) {
	string master;
	object prev, next;

	/*
	 * create linked list of clones
	 */
	if (clone) {
	    sscanf(object_name(this_object()), "%s#", master);
	    prev = find_object(master);
	    next = prev->_Q_next_clone();
	    prev->_F_next_clone(this_object());
	    next->_F_prev_clone(this_object());
	} else {
	    prev = next = this_object();
	}
	this_object()->_F_prev_clone(prev);
	this_object()->_F_next_clone(next);
    }
}

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
 * NAME:	destruct_clone()
 * DESCRIPTION:	destruct a clone
 */
atomic private int destruct_clone(object obj, object prev, object next)
{
    if (obj == this_object()) {
	/* destruct myself */
	prev->_F_next_clone(next);
	next->_F_prev_clone(prev);
	return ::destruct_object(obj);	/* always succeeds */
    } else if (::destruct_object(obj)) {
	/* destruct succeeded */
	prev->_F_next_clone(next);
	next->_F_prev_clone(prev);
	return TRUE;
    }
    return FALSE;
}

/*
 * NAME:	destruct_object()
 * DESCRIPTION:	destruct an object, with special treatment for clones
 */
static int destruct_object(mixed obj)
{
    string oname;
    object tmp;

    if (!this_object()) {
	return FALSE;
    }
    if (typeof(obj) == T_STRING) {
	tmp = find_object(obj);
	if (tmp) {
	    obj = tmp;
	}
    } else if (typeof(obj) != T_OBJECT) {
	error("Bad argument 1 for function destruct_object");
    }

    if (typeof(obj) == T_OBJECT) {
	oname = object_name(obj);
	if (sscanf(oname, "%*s/obj/") != 0) {
	    if (sscanf(oname, "%*s#") == 0) {
		/* master */
		if (obj->_Q_next_clone() != obj) {
		    error("Cannot destruct master object");
		}
	    } else {
		/* clone */
		return destruct_clone(obj, obj->_Q_prev_clone(),
				      obj->_Q_next_clone());
	    }
	}
    }

    return ::destruct_object(obj);
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
 * Functions that are disabled.
 */
static void save_object(string file)		{ return; }
static string query_ip_number(object obj)	{ return nil; }
static string query_ip_name(object obj)		{ return nil; }
