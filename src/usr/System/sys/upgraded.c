# include <status.h>
# include <type.h>
# include <kernel/access.h>
# include <kernel/kernel.h>
# include <kernel/rsrc.h>
# include <objectd.h>

inherit API_ACCESS;


object objectd;			/* object server */
string compfailed;		/* compilation of this object failed */
mapping inherited;		/* Not yet compiled lib-objects */
int factor;			/* 2nd level divisor */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize object
 */
static void create()
{
    ::create();

    objectd = find_object(OBJECTD);
    factor = status(ST_ARRAYSIZE);
}


/*
 * NAME:	recompile()
 * DESCRIPTION:	recompile leaf objects
 */
private atomic
string *recompile(string *names, mapping *leaves, mapping *depend, int fail)
{
    int i, j, sz, *status;
    string name, *objects;
    mapping failed;

    objectd->notify_compiling(this_object());

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
    do {
	int *indices, *issues;
	mapping *values, map;

	/*
	 * recompile leaf objects
	 */
	for (i = sizeof(leaves); --i >= 0; ) {
	    objects = map_indices(leaves[i]);
	    issues = map_values(leaves[i]);
	    for (j = 0, sz = sizeof(objects); j < sz; j++) {
		/* recompile a leaf object */
		name = objects[j];
		catch {
		    compile_object(name);
		} : {
		    int index, k;

		    /* recompile failed */
		    failed[compfailed] = 1;
		    compfailed = nil;

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
    if (sizeof(objects) != 0 && fail) {
	/*
	 * some objects could not be compiled -- fail atomically
	 */
	error("#" + implode(objects, "#"));
    }
    return objects;
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
 * NAME:	upgrade()
 * DESCRIPTION:	upgrade interface function
 */
mixed upgrade(string owner, string *names, int atom)
{
    if (SYSTEM()) {
	rlimits (0; -1) {
	    int i, j, sz;
	    string str, *list;
	    mapping objects, *depend, *leaves;

	    /* verify write access to objects to be upgraded directly */
	    for (i = 0, sz = sizeof(names); i < sz; i++) {
		if (!access(owner, names[i], WRITE_ACCESS)) {
		    return names[i] + ".c: Access denied.";
		}
	    }

	    /*
	     * gather information about the dependencies
	     */
	    inherited = ([ ]);
	    objects = ([ ]);
	    depend = allocate(sz);
	    for (i = sz; --i >= 0; ) {
		mapping imap, omap;

		({ imap, omap }) = objectd->query_dependents(names[i], factor);
		inherited = merge(inherited, imap);
		objects = merge(objects, omap);
		depend[i] = merge(imap, omap);
	    }
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
	    str = catch(list = recompile(names, leaves, depend, atom));
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
