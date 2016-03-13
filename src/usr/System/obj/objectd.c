# include <status.h>
# include <type.h>

# define OBJECTSERVER	"/usr/System/sys/objectd"


string creator;		/* which creator is this the database part for */
object objectd;		/* object server */
mapping included;	/* ([ path : ([ index / factor : indices ]) ]) */
mapping inherited;	/* ([ issue : ([ index / factor : indices ]) ]) */
int factor;		/* 2nd level divisor */
mapping objects;	/* ([ index : object info ]) */
mapping issues;		/* ([ object : indices ]) */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize database object
 */
static void create()
{
    objectd = find_object(OBJECTSERVER);
    factor = status(ST_ARRAYSIZE);
    included = ([ ]);
    inherited = ([ ]);
    objects = ([ ]);
    issues = ([ ]);
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
 * NAME:	add_included()
 * DESCRIPTION:	register an included file
 */
void add_included(int index, string path)
{
    if (previous_object() == objectd) {
	mapping map;

	sscanf(path, "/usr/%*s/%s", path);
	map = included[path];
	if (map) {
	    if (map[index / factor]) {
		map[index / factor] |= ({ index });
	    } else {
		map[index / factor] = ({ index });
	    }
	} else {
	    included[path] = ([ index / factor : ({ index }) ]);
	}
    }
}

/*
 * NAME:	del_included()
 * DESCRIPTION:	unregister an included file
 */
void del_included(int index, string path)
{
    if (previous_object() == objectd) {
	mapping map;

	sscanf(path, "/usr/%*s/%s", path);
	map = included[path];
	map[index / factor] -= ({ index });
	if (sizeof(map[index / factor]) == 0) {
	    map[index / factor] = nil;
	    if (map_sizeof(map) == 0) {
		included[path] = nil;
		if (map_sizeof(included) == 0 && map_sizeof(objects) == 0) {
		    destruct_object(this_object());
		}
	    }
	}
    }
}

/*
 * NAME:	query_included()
 * DESCRIPTION:	return the objects that include the given path
 */
mapping query_included(string path)
{
    if (previous_object() == objectd) {
	sscanf(path, "/usr/%*s/%s", path);
	if (included[path]) {
	    return included[path];
	}
    }
    return ([ ]);
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
		map[index / factor] |= ({ index });
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
void add_object(mixed files, int index, int *list)
{
    if (previous_object() == objectd) {
	string path;
	int i;

	if (typeof(files) == T_STRING) {
	    sscanf(files, "/usr/%*s/%s", files);
	    path = files;
	} else {
	    if (creator) {
		path = "/usr/" + creator + "/%s";
		for (i = sizeof(files); --i >= 0; ) {
		    sscanf(files[i], path, files[i]);
		}
	    }
	    path = files[0];
	}
	if (sscanf("/" + path, "%*s/lib/") == 0) {
	    /* normal object */
	    issues[path] = index;
	} else {
	    mixed issue;

	    /* lib object */
	    issue = issues[path];
	    if (issue) {
		if (typeof(issue) == T_INT) {
		    if (index != issue) {
			issues[path] = ({ index, issue });
		    }
		} else if (index != issue[0]) {
		    issues[path] = ({ index }) + issue;
		}
	    } else {
		issues[path] = index;
	    }
	}
	objects[index] = ({ files }) + list;
    }
}

/*
 * NAME:	del_object()
 * DESCRIPTION:	unregister an object
 */
int del_object(int index)
{
    if (previous_object() == objectd) {
	mixed files, issue;
	string path;

	files = objects[index][0];
	path = (typeof(files) == T_STRING) ? files : files[0];
	if (sscanf("/" + path, "%*s/lib/") == 0) {
	    /* normal object */
	    issues[path] = nil;
	    objects[index] = nil;
	    if (map_sizeof(objects) == 0 && map_sizeof(included) == 0) {
		destruct_object(this_object());
	    }
	    return TRUE;
	} else {
	    /* lib object */
	    issue = issues[path];
	    if (typeof(issue) == T_INT) {
		issues[path] = nil;
	    } else {
		issue -= ({ index });
		if (sizeof(issue) == 1) {
		    issues[path] = issue[0];
		} else {
		    issues[path] = issue;
		}
	    }

	    objects[index] = nil;
	    if (map_sizeof(objects) == 0 && map_sizeof(included) == 0) {
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
	mixed files;
	string path;

	files = objects[index][0];
	path = (typeof(files) == T_STRING) ? files : files[0];
	return (path[0] == '/') ? path : "/usr/" + creator + "/" + path;
    }
}

/*
 * NAME:	query_includes()
 * DESCRIPTION:	return the files included by the given object
 */
string *query_includes(int index)
{
    if (previous_object() == objectd) {
	mixed files;
	int i;

	files = objects[index][0];
	if (typeof(files) == T_ARRAY) {
	    files = files[1 ..];
	    for (i = sizeof(files); --i >= 0; ) {
		if (files[i][0] != '/') {
		    files[i] = "/usr/" + creator + "/" + files[i];
		}
	    }
	    return files;
	}
    }
    return ({ });
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
	    return issue[..];
	}
    }
}
