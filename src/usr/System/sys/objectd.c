# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/rsrc.h>
# include <kernel/user.h>
# include <limits.h>
# include <status.h>

inherit rsrc API_RSRC;

# define INITD		"/usr/System/initd"
# define OBJECTD	"/usr/System/sys/objectd"
# define OBJECTD_OBJ	"/usr/System/obj/objectd"


object driver;		/* driver object */
object notify;		/* object notified about changes */
mapping index_map;	/* a two-step mapping; (index / factor) -> dbase obj */
int factor;		/* 2nd level divisor */
mapping creator_map;	/* creator -> dbase object */

private void preregister_objects();

/*
 * NAME:	create()
 * DESCRIPTION:	initialize
 */
static void create()
{
    rsrc::create();

    driver = find_object(DRIVER);
    factor = status(ST_ARRAYSIZE);
    index_map = ([ ]);
    creator_map = ([ ]);

    compile_object(OBJECTD_OBJ);
    preregister_objects();
    driver->set_object_manager(this_object());
}


/*
 * NAME:	register_inherited()
 * DESCRIPTION:	register inherited objects
 */
private void register_inherited(int issue, int *list)
{
    int i, index;

    /* register inherited objects */
    for (i = sizeof(list); --i >= 0; ) {
	index = list[i];
	index_map[index / factor][index]->add_inherited(issue, index);
    }
}

/*
 * NAME:	unregister_inherited()
 * DESCRIPTION:	unregister inherited objects
 */
private void unregister_inherited(int issue, int *list)
{
    int i, index;
    mapping dbase;

    for (i = sizeof(list); --i >= 0; ) {
	index = list[i];
	dbase = index_map[index / factor];
	if (dbase[index]->del_inherited(issue, index)) {
	    /* dbase object removed */
	    dbase[index] = nil;
	    if (map_sizeof(dbase) == 0) {
		index_map[index / factor] = nil;
	    }
	}
    }
}

/*
 * NAME:	register_object()
 * DESCRIPTION:	register a new object and what it inherits
 */
private void register_object(string creator, string path, int index, int *list)
{
    int i;
    object dbobj;
    mapping dbase;

    /* get creator's dbase object */
    dbobj = creator_map[creator];
    if (!dbobj) {
	creator_map[creator] = dbobj = clone_object(OBJECTD_OBJ);
	dbobj->set_creator(creator);
    }

    /* update index mapping */
    dbase = index_map[index / factor];
    if (dbase) {
	if (dbase[index]) {
	    /* object has been recompiled: unregister old inherited */
	    unregister_inherited(index, dbobj->query_inherits(index));
	} else {
	    dbase[index] = dbobj;
	}
    } else {
	index_map[index / factor] = ([ index : dbobj ]);
    }

    /* minimize & sort list of inherited objects */
    dbase = ([ ]);
    for (i = sizeof(list); --i >= 0; ) {
	dbase[list[i]] = 1;
    }
    list = map_indices(dbase);

    /* register object in dbase object */
    dbobj->add_object(path, index, list);

    /* register inherited objects */
    register_inherited(index, list);
}

/*
 * NAME:	unregister_object()
 * DESCRIPTION:	unregister an object and what it inherits
 */
private void unregister_object(string path, int index)
{
    mapping dbase;
    object dbobj;

    dbase = index_map[index / factor];
    dbobj = dbase[index];

    /* unregister inherited objects */
    unregister_inherited(index, dbobj->query_inherits(index));

    /* unregister object */
    if (dbobj->del_object(index)) {
	/* dbase object removed */
	dbase[index] = nil;
	if (map_sizeof(dbase) == 0) {
	    index_map[index / factor] = nil;
	}
    }
}

/*
 * NAME:	prereg_inherits()
 * DESCRIPTION:	for all preregistered objects, return a list of inherited
 *		objects
 */
private string *prereg_inherits(string path)
{
    switch (path) {
    case DRIVER:
    case AUTO:
	return ({ });

    case RSRCD:	
    case ACCESSD:
    case USERD:
	return ({ AUTO });

    case API_ACCESS:
    case API_RSRC:
    case API_USER:
	return ({ AUTO });

    case LIB_CONN:
    case LIB_USER:
	return ({ AUTO });
    case LIB_WIZTOOL:
	return ({ API_ACCESS, API_RSRC, API_USER });

    case RSRCOBJ:
	return ({ AUTO });
    case BINARY_CONN:
    case TELNET_CONN:
	return ({ LIB_CONN });
    case DEFAULT_USER:
	return ({ LIB_USER, API_USER, API_ACCESS });
    case DEFAULT_WIZTOOL:
	return ({ LIB_WIZTOOL });

    case INITD:
	return ({ API_ACCESS, API_RSRC });
    case OBJECTD_OBJ:
	return ({ AUTO });
    case OBJECTD:
	return ({ API_RSRC });
    }
}

/*
 * NAME:	preregister_objects()
 * DESCRIPTION:	register objects that were created before this one
 */
private void preregister_objects()
{
    int i, sz, index;
    mixed *list;
    mapping indices;
    string name;

    /* initialize variables */
    list = ({
	DRIVER, AUTO,
	RSRCD, ACCESSD, USERD,
	API_RSRC, API_ACCESS, API_USER, 
	LIB_CONN, LIB_USER, LIB_WIZTOOL,
	RSRCOBJ, BINARY_CONN, TELNET_CONN, DEFAULT_USER, DEFAULT_WIZTOOL,
	INITD, OBJECTD, OBJECTD_OBJ
    });
    sz = sizeof(list);
    indices = ([ ]);
    for (i = sz; --i >= 0; ) {
	name = list[i];
	indices[name] = status(name, O_INDEX);
    }

    /* register objects in order of compilation, including this object */
    for (i = 0; i < sz; i++) {
	int j;
	mixed *inherits;

	name = list[i];
	inherits = prereg_inherits(name);
	for (j = sizeof(inherits); --j >= 0; ) {
	    inherits[j] = indices[inherits[j]];
	}
	register_object(driver->creator(name), name, indices[name], inherits);
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

	dbase = index_map[index / factor];
	if (dbase && (dbobj=dbase[index])) {
	    return dbobj->query_path(index);
	}
    }
}

/*
 * NAME:	query_issues()
 * DESCRIPTION:	return the registered issue for an object given by name
 */
int *query_issues(string path)
{
    if (SYSTEM()) {
	object obj;
	mixed issue;

	if (sscanf(path, "%*s/lib/") != 0) {
	    /* lib object */
	    obj = creator_map[driver->creator(path)];
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
 * NAME:	query_inherits()
 * DESCRIPTION:	return the indices of objects inherited by a given object
 */
int *query_inherits(int index)
{
    if (SYSTEM()) {
	mapping dbase;
	object dbobj;

	dbase = index_map[index / factor];
	if (dbase && (dbobj=dbase[index])) {
	    return dbobj->query_inherits(index);
	}
    }
    return ({ });
}

/*
 * NAME:	query_inherited()
 * DESCRIPTION:	return the indices of the objects that inherit a given one
 */
int **query_inherited(int index)
{
    if (SYSTEM()) {
	mapping dbase;
	object dbobj;

	if ((dbase=index_map[index / factor]) && (dbobj=dbase[index])) {
	    return map_values(dbobj->query_inherited(index));
	}
    }
    return ({ });
}


/*
 * NAME:	query_issue_deps()
 * DESCRIPTION:	collect all objects that depend on a single given object issue
 */
private void query_issue_deps(string path, int index, mapping issues,
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
		query_issue_deps(query_path(list[j]), list[j], issues,
				 inherited, leaves, factor);
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
 * DESCRIPTION:	collect the objects that depend on all issues of a given object
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
	    query_issue_deps(path, issue[i], issues, inherited, leaves, factor);
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
	int max;
	object dbobj;

	if (sscanf(path, "%*s.c/") != 0 || sscanf(path, "%*s.h/") != 0) {
	    error("Invalid object name");
	}

	/* check if there's enough space for another one */
	max = status(ST_ARRAYSIZE);
	if ((dbobj=creator_map[driver->creator(path)]) ?
	    !dbobj->test_space(path, max) : map_sizeof(creator_map) >= max) {
	    error("Out of space");
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
void compile(string owner, string path, mixed source, string inherited...)
{
    if (previous_object() == driver) {
	int i, *indices;

	i = sizeof(inherited);
	if (i != 0) {
	    /* collect indices of inherited objects */
	    indices = allocate(i);
	    do {
		--i;
		indices[i] = status(inherited[i], O_INDEX);
	    } while (i != 0);
	} else if (path == DRIVER || path == AUTO) {
	    /* no index at all */
	    indices = ({ });
	} else {
	    /* just the auto object */
	    indices = ({ status(AUTO, O_INDEX) });
	}
	register_object(owner, path, status(path, O_INDEX), indices);

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
    if (previous_object() == driver) {
	notify->compile_failed(path);
    }
}

/*
 * NAME:	destruct()
 * DESCRIPTION:	object is about to be destructed
 */
void destruct(string owner, string path)
{
    if (previous_object() == driver && sscanf(path, "%*s#") == 0) {
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
	return obj->_F_touch(func);
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
		return "/usr/System/include/std.h";
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
