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

	args = ::tls_get(0);
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
    if (path) {
	if (sscanf(path, "%*s/lib/") != 0 && status(path, O_INDEX) != nil) {
	    /* let upgrade server generate a leaf object */
	    path = UpgradeServer->generate_leaf(path);
	} else if (sscanf(path, "%*s/obj/") == 0) {
	    error("Invalid path");
	}
    }
    ::tls_set(0, args);
    return ::clone_object(path);
}

/*
 * NAME:	new_object()
 * DESCRIPTION:	create and initialize a new light-weight object
 */
static object new_object(string path, mixed args...)
{
    if (path) {
	if (sscanf(path, "%*s/lib/") != 0 && status(path, O_INDEX) != nil) {
	    /* let upgrade server generate a leaf object */
	    path = UpgradeServer->generate_leaf(path);
	} else if (sscanf(path, "%*s/data/") == 0) {
	    error("Invalid path");
	}
    }
    ::tls_set(0, args);
    return ::new_object(path);
}

/*
 * NAME:	copy_object()
 * DESCRIPTION:	copy a light-weight object
 */
static object copy_object(object obj)
{
    if (!obj || sscanf(object_name(obj), "%*s#-1") == 0) {
	error("Not a light-weight object");
    }
    return ::new_object(obj);
}

/*
 * NAME:	compile_object()
 * DESCRIPTION:	compile an object
 */
static object compile_object(string path)
{
    object obj;

    if (path) {
	if (sscanf(path, "%*s/@@@/") != 0) {
	    error("Cannot compile leaf object");
	}
	if (sscanf(path, "%*s/lib/") + sscanf(path, "%*s/obj/") +
	    sscanf(path, "%*s/data/") > 1) {
	    error("Ambiguous object");
	}
    }
    obj = ::compile_object(path);
    if (obj) {
	call_other(obj, "???");
    }
}


/*
 * NAME:	tls_set()
 * DESCRIPTION:	set TLS value
 */
static void tls_set(string index, mixed value)
{
    ::tls_set(index, value);
}

/*
 * NAME:	tls_get()
 * DESCRIPTION:	get TLS value
 */
static mixed tls_get(string index)
{
    return ::tls_get(index);
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
