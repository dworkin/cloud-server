# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/tls.h>
# include <status.h>
# include <type.h>

inherit access API_ACCESS;
inherit tls API_TLS;

# define ObjectServer	"/usr/System/sys/objectd"


object objectd;			/* object server */
string compfailed;		/* compilation of this object failed */
mapping inherited;		/* Not yet compiled lib-objects */
mapping *patching;		/* objects left to patch */
int factor;			/* 2nd level divisor */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize object
 */
static void create()
{
    access::create();
    tls::create();

    objectd = find_object(ObjectServer);
    factor = status(ST_ARRAYSIZE);
}


/*
 * NAME:	merge()
 * DESCRIPTION:	merge two mapping structures (changes first argument)
 */
private mapping merge(mapping m1, mapping m2)
{
    int i, index, *indices;
    mapping *values;

    indices = map_indices(m2);
    values = map_values(m2);
    for (i = sizeof(indices); --i >= 0; ) {
	index = indices[i];
	if (m1[index]) {
	    m1[index] += values[i];
	} else {
	    m1[index] = values[i];
	}
    }

    return m1;
}

/*
 * NAME:	dependencies()
 * DESCRIPTION:	find the dependencies for a list of objects, as library objects,
 *		leaf objects, and total dependencies per object in the list.
 */
private mixed *dependencies(string *names)
{
    mapping libs, leaves, *all;
    int i, sz;

    libs = ([ ]);
    leaves = ([ ]);
    sz = sizeof(names);
    all = allocate(sz);
    for (i = sz; --i >= 0; ) {
	mapping imap, omap;

	({ imap, omap }) = objectd->query_dependents(names[i], factor);
	libs = merge(libs, imap);
	leaves = merge(leaves, omap);
	all[i] = merge(imap, omap);
    }

    return ({ libs, leaves, all });
}

/*
 * NAME:	touch_clones()
 * DESCRIPTION:	prepare to patch clones
 */
private mixed touch_clones(string master)
{
    mixed max;
    int i;
    object obj;

    max = nil;
    master += "#";
    for (i = status(ST_OTABSIZE); --i >= 0; ) {
	obj = find_object(master + i);
	if (obj) {
	    if (max == nil && !(obj <- "~/lib/auto")) {
		return nil;
	    }

	    call_touch(obj);
	    max = i;
	}
    }

    return max;
}

/*
 * NAME:	next()
 * DESCRIPTION:	find the next object to patch
 */
private object next()
{
    mapping map;
    string name;
    int clone;
    object obj;

    while (sizeof(patching) != 0) {
	map = patching[0];
	if (map_sizeof(map) == 0) {
	    patching = patching[1 ..];
	} else {
	    name = map_indices(map)[0];
	    if (sscanf(name, "%*s/obj/") != 0) {
		/*
		 * find the next clone
		 */
		clone = map[name];
		while (clone >= 0) {
		    obj = find_object(name + "#" + clone);
		    --clone;
		    if (obj) {
			map[name] = clone;
			return obj;
		    }
		}
		map[name] = nil;
	    } else {
		/*
		 * find a master object
		 */
		obj = find_object(name);
		map[name] = nil;
		if (obj) {
		    return obj;
		}
	    }
	}
    }

    patching = nil;
    return nil;
}

/*
 * NAME:	patcher()
 * DESCRIPTION:	patch one object, and start a callout for the next one
 */
static void patcher(object patchtool)
{
    object obj;

    obj = next();

    if (obj) {
	call_out("patcher", 0, patchtool);
	patchtool->do_patch(obj);
    }
}

/*
 * NAME:	recompile()
 * DESCRIPTION:	recompile leaf objects
 */
private atomic
string *recompile(string *names, mapping *leaves, mapping *depend, int atom,
		  int patch)
{
    int i, j, sz, *status;
    string name, *objects;
    mapping failed;

    objectd->notify_compiling(this_object());
    set_tlvar(0, TRUE);

    /*
     * Destruct inherited lib objects among the ones that are being upgraded.
     */
    for (i = sizeof(names); --i >= 0; ) {
	name = names[i];
	if (sscanf(name, "%*s/lib/") != 0 && (status=status(name)) &&
	    sizeof(objectd->query_inherited(status[O_INDEX])) != 0) {
	    destruct_object(name);
	}
    }

    failed = ([ ]);
    if (patch) {
	patching = leaves;
    }
    do {
	int *indices, *issues;
	mapping *values, map;
	object obj;

	/*
	 * recompile leaf objects
	 */
	for (i = sizeof(leaves); --i >= 0; ) {
	    map = leaves[i];
	    objects = map_indices(map);
	    issues = map_values(map);
	    for (j = 0, sz = sizeof(objects); j < sz; j++) {
		/* recompile a leaf object */
		name = objects[j];
		catch {
		    compile_object(name);
		    if (patch) {
			/*
			 * patch objects after upgrade
			 */
			if (sscanf(name, "%*s/obj/") != 0) {
			    if (sscanf(name, "/kernel/%*s") == 0 &&
				name != "/usr/System/obj/objectd") {
				map[objects[j]] = touch_clones(name);
				continue;
			    }
			} else if ((obj=find_object(name)) &&
				   obj <- "~/lib/auto") {
			    call_touch(obj);
			    continue;
			}
			map[objects[j]] = nil;
		    }
		} : {
		    int index, k;

		    /* recompile failed */
		    failed[compfailed] = 1;
		    compfailed = nil;
		    patch = FALSE;

		    /* take note of upgraded objects that are inherited */
		    index = issues[j] / factor;
		    for (k = sizeof(depend); --k >= 0; ) {
			map = depend[k][index];
			if (map && map[name]) {
			    names[k] = nil;
			}
		    }
		}
	    }
	}
	patch = FALSE;

	/*
	 * get newly exposed leaves
	 */
	leaves = ({ });
	indices = map_indices(inherited);
	values = map_values(inherited);
	for (i = sizeof(indices); --i >= 0; ) {
	    map = values[i][..];
	    objects = map_indices(values[i]);
	    issues = map_values(values[i]);
	    for (j = sizeof(objects); --j >= 0; ) {
		if (sizeof(objectd->query_inherited(issues[j])) != 0) {
		    map[objects[j]] = nil;
		}
	    }
	    if (map_sizeof(map) != 0) {
		leaves += ({ map });
		inherited[indices[i]] -= map_indices(map);
	    }
	}
    } while (sizeof(leaves) != 0);

    inherited = nil;
    objectd->notify_compiling(nil);

    objects = map_indices(failed);
    if (sizeof(objects) != 0 && atom) {
	/*
	 * some objects could not be compiled -- fail atomically
	 */
	error("#" + implode(objects, "#"));
    } else if (patching) {
	/*
	 * patch affected objects through callout
	 */
	call_out("patcher", 0, previous_object());
    }
    return objects;
}

/*
 * NAME:	upgrade()
 * DESCRIPTION:	upgrade interface function
 */
mixed upgrade(string owner, string *names, int atom, int patch)
{
    if (SYSTEM()) {
	rlimits (0; -1) {
	    int i, j, sz;
	    string str, *list;
	    mapping objects, *depend, *leaves;

	    if (patch && patching) {
		return "Still patching after previous upgrade.";
	    }

	    /* verify write access to objects to be upgraded directly */
	    for (i = 0, sz = sizeof(names); i < sz; i++) {
		if (!access(owner, names[i], WRITE_ACCESS)) {
		    return names[i] + ".c: Access denied.";
		}
	    }

	    /*
	     * gather information about the dependencies
	     */
	    ({ inherited, objects, depend }) = dependencies(names);
	    if (map_sizeof(inherited) + map_sizeof(objects) == 0) {
		inherited = nil;
		return "No existing issues.";
	    }

	    /*
	     * check if leaves can be recompiled
	     */
	    leaves = map_values(objects);
	    for (i = sizeof(leaves); --i >= 0; ) {
		list = map_indices(leaves[i]);
		for (j = sizeof(list); --j >= 0; ) {
		    str = list[j] + ".c";
		    if (!access(owner, str, WRITE_ACCESS)) {
			inherited = nil;
			return str + ": Access denied.";
		    }
		    if (!file_info(str)) {
			inherited = nil;
			return str + ": Missing source file.";
		    }
		}
	    }

	    /*
	     * recompile leaf objects
	     */
	    str = catch(list = recompile(names, leaves, depend, atom, patch));
	    if (!str) {
		/* return failed list */
		return list;
	    } else if (str[0] == '#') {
		/* failed list returned in error message */
		return explode(str, "#");
	    } else {
		/* return error */
		return str;
	    }
	}
    }
}


/*
 * NAME:	compile_failed()
 * DESCRIPTION:	attempt to compile object failed
 */
void compile_failed(string path)
{
    if (previous_object() == objectd && !compfailed) {
	compfailed = path;
    }
}

/*
 * NAME:	destruct()
 * DESCRIPTION:	check if a lib object is about to be destructed
 */
void destruct(string path)
{
    if (previous_object() == objectd && sscanf(path, "%*s/lib/") != 0) {
	inherited[status(path, O_INDEX) / factor][path] = nil;
    }
}

/*
 * NAME:	query_upgrading()
 * DESCRIPTION:	upgrading during this task?
 */
int query_upgrading()
{
    return !!get_tlvar(0);
}
