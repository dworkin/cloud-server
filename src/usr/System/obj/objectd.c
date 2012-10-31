# include <status.h>
# include <type.h>
# include <objectd.h>

string creator;		/* which creator is this the database part for */
object objectd;		/* object server */
mapping inherited;	/* ([ issue : ([ index / factor : indices ]) ]) */
int factor;		/* 2nd level divisor */
mapping objects;	/* ([ index : object info ]) */
mapping issues;		/* ([ object : indices ]) */

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
	issues = ([ ]);
    }
}

/*
 * NAME:	set_creator()
 * DESCRIPTION:	set the creator of all objects in this database
 */
void set_creator(string str)
{
    if (previous_object() == objectd) {
	creator = str;
    }
}

/*
 * NAME:	test_space()
 * DESCRIPTION:	see if there's room for another one
 */
int test_space(string path, int max)
{
    if (previous_object() == objectd) {
	mixed issue;

	if (map_sizeof(objects) >= max) {
	    return FALSE;
	}
	sscanf(path, "/usr/%*s/%s", path);
	issue = issues[path];
	if (typeof(issue) == T_ARRAY && sizeof(issue) >= max) {
	    return FALSE;
	}
    }
    return TRUE;
}

/*
 * NAME:	add_inherited()
 * DESCRIPTION:	register an inherited object
 */
void add_inherited(int index, int issue)
{
    if (previous_object() == objectd) {
	mapping map;

	map = inherited[issue];
	if (map) {
	    if (map[index / factor]) {
		map[index / factor] += ({ index });
	    } else {
		map[index / factor] = ({ index });
	    }
	} else {
	    inherited[issue] = ([ index / factor : ({ index }) ]);
	}
    }
}

/*
 * NAME:	del_inherited()
 * DESCRIPTION:	unregister an inherited object
 */
int del_inherited(int index, int issue)
{
    if (previous_object() == objectd) {
	mapping map;

	map = inherited[issue];
	map[index / factor] -= ({ index });
	if (sizeof(map[index / factor]) == 0) {
	    map[index / factor] = nil;
	    if (map_sizeof(map) == 0) {
		inherited[issue] = nil;
		return (objects[issue] == nil);
	    }
	}
    }
    return FALSE;
}

/*
 * NAME:	query_inherited()
 * DESCRIPTION:	return the objects that inherit the given object
 */
mapping query_inherited(int issue)
{
    if (previous_object() == objectd && inherited[issue]) {
	return inherited[issue];
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
	    issues[obj] = index;
	} else {
	    mixed issue;

	    /* lib object */
	    sscanf(obj, "/usr/%*s/%s", obj);
	    issue = issues[obj];
	    if (issue) {
		if (typeof(issue) == T_INT) {
		    if (index != issue) {
			issues[obj] = ({ index, issue });
		    }
		} else if (index != issue[0]) {
		    issues[obj] = ({ index }) + issue;
		}
	    } else {
		issues[obj] = index;
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
	mixed obj, issue;

	obj = objects[index][0];
	if (typeof(obj) == T_OBJECT) {
	    /* normal object */
	    issues[obj] = nil;
	    objects[index] = nil;
	    if (map_sizeof(objects) == 0) {
		destruct_object(this_object());
	    }
	    return TRUE;
	} else {
	    /* lib object */
	    issue = issues[obj];
	    if (typeof(issue) == T_INT) {
		issues[obj] = nil;
	    } else {
		issue -= ({ index });
		if (sizeof(issue) == 1) {
		    issues[obj] = issue[0];
		} else {
		    issues[obj] = issue;
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
	    return (path[0] == '/') ? path : "/usr/" + creator + "/" + path;
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
	mixed issue;

	sscanf(path, "/usr/%*s/%s", path);
	issue = issues[path];
	if (!issue) {
	    return ({ });
	} else if (typeof(issue) == T_INT) {
	    return ({ issue });
	} else {
	    return issue;
	}
    }
}
