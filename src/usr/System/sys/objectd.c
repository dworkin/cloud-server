# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/rsrc.h>
# include <kernel/user.h>
# include <limits.h>
# include <status.h>

# define INIT		"/usr/System/initd"
# define OBJECTSERVER	"/usr/System/sys/objectd"
# define OBJDBASE	"/usr/System/obj/objectd"


object driver;		/* driver object */
object notify;		/* object notified about changes */
int factor;		/* 2nd level divisor */
mapping index2db;	/* ([ index / factor : dbase object ]) */
mapping creator2db;	/* ([ creator : dbase object ]) */

private void preregister_objects();

/*
 * NAME:	create()
 * DESCRIPTION:	initialize
 */
static void create()
{
    driver = find_object(DRIVER);
    factor = status(ST_ARRAYSIZE);
    index2db = ([ ]);
    creator2db = ([ ]);

    compile_object(OBJDBASE);
    preregister_objects();
    driver->set_object_manager(this_object());
}


/*
 * NAME:	register_included()
 * DESCRIPTION:	register included files
 */
private void register_included(int index, string *includes)
{
    int i;
    string creator;
    object dbobj;

    for (i = sizeof(includes); --i >= 0; ) {
	creator = driver->creator(includes[i]);
	dbobj = creator2db[creator];
	if (!dbobj) {
	    creator2db[creator] = dbobj = clone_object(OBJDBASE);
	    dbobj->set_creator(creator);
	}
	dbobj->add_included(index, includes[i]);
    }
}

/*
 * NAME:	unregister_included()
 * DESCRIPTION:	unregister included files
 */
private void unregister_included(int index, string *includes)
{
    int i;

    for (i = sizeof(includes); --i >= 0; ) {
	creator2db[driver->creator(includes[i])]->del_included(index,
							       includes[i]);
    }
}

/*
 * NAME:	register_inherited()
 * DESCRIPTION:	register inherited objects
 */
private void register_inherited(int issue, int *inherits)
{
    int i, index;

    /* register inherited objects */
    for (i = sizeof(inherits); --i >= 0; ) {
	index = inherits[i];
	index2db[index / factor][index]->add_inherited(issue, index);
    }
}

/*
 * NAME:	unregister_inherited()
 * DESCRIPTION:	unregister inherited objects
 */
private void unregister_inherited(int issue, int *inherits)
{
    int i, index;
    mapping dbase;

    for (i = sizeof(inherits); --i >= 0; ) {
	index = inherits[i];
	dbase = index2db[index / factor];
	if (dbase[index]->del_inherited(issue, index)) {
	    /* index completely removed */
	    dbase[index] = nil;
	    if (map_sizeof(dbase) == 0) {
		index2db[index / factor] = nil;
	    }
	}
    }
}

/*
 * NAME:	register_object()
 * DESCRIPTION:	register a new object and what it inherits
 */
private void register_object(string creator, string path, string *includes,
			     string *inherits)
{
    int index, i;
    object dbobj;
    mapping dbase;
    int *issues;

    /* get creator's dbase object */
    dbobj = creator2db[creator];
    if (!dbobj) {
	creator2db[creator] = dbobj = clone_object(OBJDBASE);
	dbobj->set_creator(creator);
    }

    /* update index mapping */
    index = status(path, O_INDEX);
    dbase = index2db[index / factor];
    if (dbase) {
	if (dbase[index]) {
	    /* object recompiled: unregister old inherited and included */
	    unregister_inherited(index, dbobj->query_inherits(index));
	    unregister_included(index, dbobj->query_includes(index));
	} else {
	    dbase[index] = dbobj;
	}
    } else {
	index2db[index / factor] = ([ index : dbobj ]);
    }

    /* obtain list of inherited issues */
    for (i = sizeof(inherits), issues = allocate_int(i); --i >= 0; ) {
	issues[i] = status(inherits[i], O_INDEX);
    }

    /* register object in dbase object */
    dbobj->add_object((sizeof(includes) != 0) ? ({ path }) + includes : path,
		      index, issues);

    /* register included and inherited */
    register_included(index, includes);
    register_inherited(index, issues);
}

/*
 * NAME:	unregister_object()
 * DESCRIPTION:	unregister an object and what it inherits
 */
private void unregister_object(string path, int index)
{
    mapping dbase;
    object dbobj;

    dbase = index2db[index / factor];
    dbobj = dbase[index];

    /* unregister inherited and included */
    unregister_inherited(index, dbobj->query_inherits(index));
    unregister_included(index, dbobj->query_includes(index));

    /* unregister object */
    if (dbobj->del_object(index)) {
	/* dbase object removed */
	dbase[index] = nil;
	if (map_sizeof(dbase) == 0) {
	    index2db[index / factor] = nil;
	}
    }
}

/*
 * NAME:	preregister_includes()
 * DESCRIPTION:	for all preregistered objects, return a list of included files
 */
private string *preregister_includes(string path)
{
    switch (path) {
    case DRIVER:
    case AUTO:
	return ({
	    "/include/kernel/access.h",
	    "/include/kernel/kernel.h",
	    "/include/kernel/rsrc.h",
	    "/include/kernel/user.h",
	    "/include/status.h",
	    "/include/trace.h",
	    "/include/type.h"
	});

    case RSRCD:
    case API_RSRC:
	return ({
	    "/include/kernel/kernel.h",
	    "/include/kernel/rsrc.h",
	    "/include/type.h"
	});

    case ACCESSD:
	return ({
	    "/include/kernel/access.h",
	    "/include/kernel/kernel.h",
	    "/include/type.h"
	});

    case USERD:
	return ({
	    "/include/kernel/kernel.h",
	    "/include/kernel/rsrc.h",
	    "/include/kernel/user.h"
	});

    case API_ACCESS:
	return ({
	    "/include/kernel/access.h",
	    "/include/kernel/kernel.h"
	});

    case API_USER:
    case LIB_CONN:
    case LIB_USER:
    case BINARY_CONN:
    case TELNET_CONN:
    case DEFAULT_WIZTOOL:
	return ({
	    "/include/kernel/kernel.h",
	    "/include/kernel/user.h"
	});

    case LIB_WIZTOOL:
	return ({
	    "/include/kernel/access.h",
	    "/include/kernel/kernel.h",
	    "/include/kernel/rsrc.h",
	    "/include/kernel/user.h",
	    "/include/status.h",
	    "/include/type.h"
	});

    case RSRCOBJ:
	return ({
	    "/include/kernel/kernel.h",
	    "/include/kernel/rsrc.h",
	    "/include/status.h",
	    "/include/trace.h",
	    "/include/type.h"
	});

    case DEFAULT_USER:
	return ({
	    "/include/kernel/access.h",
	    "/include/kernel/kernel.h",
	    "/include/kernel/user.h"
	});

    case INIT:
	return ({
	    "/include/kernel/access.h",
	    "/include/kernel/kernel.h",
	    "/include/kernel/rsrc.h",
	    "/include/kernel/user.h"
	});

    case OBJECTSERVER:
	return ({
	    "/include/kernel/access.h",
	    "/include/kernel/kernel.h",
	    "/include/kernel/rsrc.h",
	    "/include/kernel/user.h",
	    "/include/limits.h",
	    "/include/status.h"
	});

    case OBJDBASE:
	return ({
	    "/include/status.h",
	    "/include/type.h"
	});
    }
}

/*
 * NAME:	preregister_inherits()
 * DESCRIPTION:	for all preregistered objects, return a list of inherited
 *		objects
 */
private string *preregister_inherits(string path)
{
    switch (path) {
    case DRIVER:
    case AUTO:
	return ({ });

    case RSRCD:
    case ACCESSD:
    case USERD:
    case API_ACCESS:
    case API_RSRC:
    case API_USER:
    case LIB_CONN:
    case LIB_USER:
    case RSRCOBJ:
    case OBJECTSERVER:
    case OBJDBASE:
	return ({ AUTO });

    case LIB_WIZTOOL:
	return ({ API_ACCESS, API_RSRC, API_USER });

    case BINARY_CONN:
    case TELNET_CONN:
	return ({ LIB_CONN });

    case DEFAULT_USER:
	return ({ LIB_USER, API_USER, API_ACCESS });

    case DEFAULT_WIZTOOL:
	return ({ LIB_WIZTOOL });

    case INIT:
	return ({ API_ACCESS, API_RSRC });
    }
}

/*
 * NAME:	preregister_objects()
 * DESCRIPTION:	register objects that were created before this one
 */
private void preregister_objects()
{
    mixed *list;
    int i, sz;
    string name;

    /* register objects in order of compilation, including this object */
    list = ({
	DRIVER, AUTO,
	RSRCD, ACCESSD, USERD,
	API_RSRC, API_ACCESS, API_USER, 
	LIB_CONN, LIB_USER, LIB_WIZTOOL,
	RSRCOBJ, BINARY_CONN, TELNET_CONN, DEFAULT_USER, DEFAULT_WIZTOOL,
	INIT, OBJECTSERVER, OBJDBASE
    });

    for (i = 0, sz = sizeof(list); i < sz; i++) {
	name = list[i];
	register_object(driver->creator(name), name, preregister_includes(name),
			preregister_inherits(name));
    }
}

/*
 * NAME:	query_path()
 * DESCRIPTION:	return the name of an object given by index
 */
string query_path(int index)
{
    if (SYSTEM()) {
	mapping dbase;
	object dbobj;

	dbase = index2db[index / factor];
	if (dbase && (dbobj=dbase[index])) {
	    return dbobj->query_path(index);
	}
    }
}

/*
 * NAME:	query_issues()
 * DESCRIPTION:	return the registered issues for an object given by name
 */
int *query_issues(string path)
{
    if (SYSTEM()) {
	object obj;
	mixed issue;

	if (sscanf(path, "%*s/lib/") != 0) {
	    /* lib object */
	    obj = creator2db[driver->creator(path)];
	    if (obj) {
		return obj->query_issues(path);
	    }
	} else {
	    /* normal object: has only one issue */
	    issue = status(path, O_INDEX);
	    if (issue != nil) {
		return ({ issue });
	    }
	}
    }
    return ({ });
}

/*
 * NAME:	query_includes()
 * DESCRIPTION:	return the files included by a given object
 */

string *query_includes(int index)
{
    if (SYSTEM()) {
	mapping dbase;
	object dbobj;

	dbase = index2db[index / factor];
	if (dbase && (dbobj=dbase[index])) {
	    return dbobj->query_includes(index);
	}
    }
    return ({ });
}

/*
 * NAME:	query_included()
 * DESCRIPTION:	return the indices of objects that include a given file
 */
int **query_included(string path)
{
    if (SYSTEM()) {
	object dbobj;

	dbobj = creator2db[driver->creator(path)];
	if (dbobj) {
	    return map_values(dbobj->query_included(path));
	}
    }
    return ({ });
}

/*
 * NAME:	query_inherits()
 * DESCRIPTION:	return the indices of objects inherited by a given object
 */
int *query_inherits(int index)
{
    if (SYSTEM()) {
	mapping dbase;
	object dbobj;

	dbase = index2db[index / factor];
	if (dbase && (dbobj=dbase[index])) {
	    return dbobj->query_inherits(index);
	}
    }
    return ({ });
}

/*
 * NAME:	query_inherited()
 * DESCRIPTION:	return the indices of objects that inherit a given one
 */
int **query_inherited(int index)
{
    if (SYSTEM()) {
	mapping dbase;
	object dbobj;

	if ((dbase=index2db[index / factor]) && (dbobj=dbase[index])) {
	    return map_values(dbobj->query_inherited(index));
	}
    }
    return ({ });
}


/*
 * NAME:	gather_deps()
 * DESCRIPTION:	gather all objects that depend on a single given object issue
 */
private void gather_deps(string path, int index, mapping issues,
			 mapping inherited, mapping leaves, int factor)
{
    mapping map;
    int i, j, **lists, *list;

    if (sscanf(path, "%*s/lib/")) {
	/*
	 * lib objects are stored in the 'inherited' structure as strings
	 */
	map = issues[index / factor];
	if (map) {
	    if (map[index]) {
		return;		/* already dealt with */
	    }
	    map[index] = TRUE;
	} else {
	    issues[index / factor] = ([ index : TRUE ]);
	}
	if (status(path, O_INDEX) == index) {
	    map = inherited[index / factor];
	    if (map) {
		map[path] = index;
	    } else {
		inherited[index / factor] = ([ path : index ]);
	    }
	}
	lists = query_inherited(index);
	for (i = sizeof(lists); --i >= 0; ) {
	    list = lists[i];
	    for (j = sizeof(list); --j >= 0; ) {
		gather_deps(query_path(list[j]), list[j], issues, inherited,
			    leaves, factor);
	    }
	}
    } else {
	/*
	 * Not a lib object, so it must be a leaf object, which is stored in the
	 * 'leaves' structure as an object.
	 */
	map = leaves[index / factor];
	if (map) {
	    map[path] = index;
	} else {
	    leaves[index / factor] = ([ path : index ]);
	}
    }
}

/*
 * NAME:	query_dependents()
 * DESCRIPTION:	Return the objects that depend on all issues of a given object,
 *		separated into inheritables and leaves.
 *		Datastructure: ([ index / factor : ([ path : index ]) ])
 */
mapping *query_dependents(string path, int factor)
{
    if (SYSTEM()) {
	int i, *issue;
	mapping issues, inherited, leaves;

	/* collect dependents for all issues of object */
	issues = ([ ]);
	inherited = ([ ]);
	leaves = ([ ]);
	issue = query_issues(path);
	for (i = sizeof(issue); --i >= 0; ) {
	    gather_deps(path, issue[i], issues, inherited, leaves, factor);
	}

	return ({ inherited, leaves });
    }
}


/*
 * NAME:	notify_compiling()
 * DESCRIPTION:	set the object to be notified of changes
 */
void notify_compiling(object obj)
{
    if (SYSTEM()) {
	notify = obj;
    }
}

/*
 * NAME:	compiling()
 * DESCRIPTION:	an object is about to be compiled
 */
void compiling(string path)
{
    if (previous_object() == driver) {
	if (sscanf(path, "%*s.c/") != 0 || sscanf(path, "%*s.h/") != 0) {
	    error("Invalid object name");
	}

	if (notify) {
	    notify->compiling(path);
	}
    }
}

/*
 * NAME:	compile()
 * DESCRIPTION:	an object has been compiled
 */
void compile(string owner, string path, mapping source, string inherits...)
{
    if (previous_object() == driver) {
	mapping undefined;
	string *includes;

	if (sscanf(path, "%*s/lib/") == 0) {
	    undefined = status(path, O_UNDEFINED);
	    if (undefined) {
		error("Function " + map_values(undefined)[0][0] +
		      " not implemented by \"" + path + "\"");
	    }
	}

	source[path + ".c"] = source["/include/AUTO"] = nil;
	includes = map_indices(source);
	if (sizeof(inherits) == 0 && path != DRIVER && path != AUTO) {
	    /* just the auto object */
	    inherits = ({ AUTO });
	}

	catch {
	    register_object(owner, path, includes, inherits);
	} : error("Out of space for object \"" + path + "\"");

	if (notify) {
	    notify->compile(path);
	}
    }
}

/*
 * NAME:	compile_failed()
 * DESCRIPTION:	attempt to compile object failed
 */
void compile_failed(string owner, string path)
{
    if (previous_object() == driver && notify) {
	notify->compile_failed(path);
    }
}

/*
 * NAME:	destruct()
 * DESCRIPTION:	object is about to be destructed
 */
void destruct(string owner, string path)
{
    if (previous_object() == driver) {
	if (sscanf(path, "%*s/lib/") == 0) {
	    unregister_object(path, status(path, O_INDEX));
	}

	if (notify) {
	    notify->destruct(path);
	}
    }
}

/*
 * NAME:	touch()
 * DESCRIPTION:	an object flagged with call_touch() is touched
 */
int touch(object obj, string func)
{
    if (previous_object() == driver) {
	return obj->_F_touch();
    }
    return FALSE;
}

/*
 * NAME:	remove_program()
 * DESCRIPTION:	the last reference to an object's program has been removed
 */
void remove_program(string owner, string path, int timestamp, int index)
{
    if (previous_object() == driver && sscanf(path, "%*s/lib/")) {
	/* only for lib objects */
	unregister_object(path, index);
    }
}

/*
 * NAME:	inherit_program()
 * DESCRIPTION:	handle inheritance
 */
mixed inherit_program(string from, string path, int priv)
{
    if (previous_object() == driver) {
	return path;
    }
}

/*
 * NAME:	include_file()
 * DESCRIPTION:	handle an include file
 */
mixed include_file(string compiled, string from, string path)
{
    if (previous_object() == driver) {
	int len;

	if (path == "/include/AUTO" && from == "/include/std.h") {
	    if (driver->creator(compiled) != "System") {
		/*
		 * special include file
		 */
		return ({ "inherit \"/usr/System/lib/auto\";\n" });
	    } else {
		return ({ "" });
	    }
	}

	/* checks */
	len = strlen(path);
	if (path != "/include/AUTO" &&
	    (sscanf(path, "%*s.c/") != 0 || sscanf(path, "%*s.h/") != 0 ||
	     len < 2 || path[len - 2 ..] != ".h")) {
	    error("Invalid include file");
	}
	return path;
    }
}
