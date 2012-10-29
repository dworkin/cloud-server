# include <status.h>
# include <type.h>
# include <objectd.h>

string dbase_creator;	/* which creator is this the database part for */
object objectd;		/* object server */
mapping	inherited;	/* (index / factor) --> inherited */
int factor;		/* 2nd level divisor */
mapping	objects;	/* ([ index : object info ]) */
mapping	names;		/* ([ object : index/indices ]) */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize database object
 */
static void create(int clone)
{
    if (clone) {
	objectd = find_object(OBJECTD);
	factor = status(ST_ARRAYSIZE);
	inherited = ([ ]);
	objects = ([ ]);
	names = ([ ]);
    }
}

/*
 * NAME:	set_creator()
 * DESCRIPTION:	set the creator of all objects in this database
 */
void set_creator(string str)
{
    if (previous_object() == objectd) {
	dbase_creator = str;
    }
}

/*
 * NAME:	test_space()
 * DESCRIPTION:	see if there's room for another one
 */
int test_space(string path, int max)
{
    if (previous_object() == objectd) {
	mixed issues;

	if (map_sizeof(objects) >= max) {
	    return FALSE;
	}
	sscanf(path, "/usr/%*s/%s", path);
	if (typeof(issues=names[path]) == T_ARRAY && sizeof(issues) >= max) {
	    return FALSE;
	}
    }
    return TRUE;
}

/*
 * NAME:	add_inherited()
 * DESCRIPTION:	register an inherited object
 */
void add_inherited(int index, int iindex)
{
    if (previous_object() == objectd) {
	mapping map;

	map = inherited[iindex];
	if (map) {
	    if (map[index / factor]) {
		map[index / factor] += ({ index });
	    } else {
		map[index / factor] = ({ index });
	    }
	} else {
	    inherited[iindex] = ([ index / factor : ({ index }) ]);
	}
    }
}

/*
 * NAME:	del_inherited()
 * DESCRIPTION:	unregister an inherited object
 */
int del_inherited(int index, int iindex)
{
    if (previous_object() == objectd) {
	mapping map;

	map = inherited[iindex];
	map[index / factor] -= ({ index });
	if (sizeof(map[index / factor]) == 0) {
	    map[index / factor] = nil;
	    if (map_sizeof(map) == 0) {
		inherited[iindex] = nil;
		return (objects[iindex] == nil);
	    }
	}
    }
    return FALSE;
}

/*
 * NAME:	query_inherited()
 * DESCRIPTION:	return the objects that inherit the given object
 */
mapping query_inherited(int iindex)
{
    if (previous_object() == objectd && inherited[iindex]) {
	return inherited[iindex];
    }
    return ([ ]);
}

/*
 * NAME:	add_object()
 * DESCRIPTION:	register an object
 */
void add_object(mixed obj, int index, int *list)
{
    if (previous_object() == objectd) {
	if (typeof(obj) == T_OBJECT) {
	    /* normal object */
	    names[obj] = index + 1;
	} else {
	    mixed issues;

	    /* lib object */
	    sscanf(obj, "/usr/%*s/%s", obj);
	    issues = names[obj];
	    if (issues) {
		if (typeof(issues) == T_INT) {
		    if (index != issues - 1) {
			names[obj] = ({ index, issues - 1 });
		    }
		} else {
		    if (index != issues[0]) {
			names[obj] = ({ index }) + issues;
		    }
		}
	    } else {
		names[obj] = index + 1;
	    }
	}
	objects[index] = ({ obj }) + list;
    }
}

/*
 * NAME:	del_object()
 * DESCRIPTION:	unregister an object
 */
int del_object(int index)
{
    if (previous_object() == objectd) {
	mixed obj;
	mixed issues;

	obj = objects[index][0];
	if (typeof(obj) == T_OBJECT) {
	    /* normal object */
	    names[obj] = nil;
	    objects[index] = nil;
	    if (map_sizeof(objects) == 0) {
		destruct_object(this_object());
	    }
	    return TRUE;
	} else {
	    /* lib object */
	    issues = names[obj];
	    if (typeof(issues) == T_INT) {
		names[obj] = nil;
	    } else {
		issues -= ({ index });
		if (sizeof(issues) == 1) {
		    names[obj] = issues[0] + 1;
		} else {
		    names[obj] = issues;
		}
	    }

	    objects[index] = nil;
	    if (map_sizeof(objects) == 0) {
		destruct_object(this_object());
	    }
	    return (inherited[index] == nil);
	}
    }
}

/*
 * NAME:	query_path()
 * DESCRIPTION:	return the name of an object
 */
string query_path(int index)
{
    if (previous_object() == objectd) {
	mixed path;

	path = objects[index][0];
	if (typeof(path) == T_OBJECT) {
	    return object_name(path);
	} else {
	    return path[0] == '/' ? path : "/usr/" + dbase_creator + "/" + path;
	}
    }
}

/*
 * NAME:	query_inherits()
 * DESCRIPTION:	return the objects inherited by the given object
 */
int *query_inherits(int index)
{
    if (previous_object() == objectd) {
	return objects[index][1 ..];
    }
}

/*
 * NAME:	query_issues()
 * DESCRIPTION:	return the issues registered for a given object
 */
int *query_issues(string path)
{
    if (previous_object() == objectd) {
	mixed issues;

	sscanf(path, "/usr/%*s/%s", path);
	issues = names[path];
	if (!issues) {
	    return ({ });
	} else if (typeof(issues) == T_INT) {
	    return ({ issues - 1 });
	} else {
	    return issues;
	}
    }
}
