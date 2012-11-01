# include <status.h>
# include <type.h>
# include <kernel/access.h>
# include <kernel/kernel.h>
# include <kernel/rsrc.h>
# include <objectd.h>

inherit API_ACCESS;


object objectd;			/* object server */
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
 * DESCRIPTION:	recompile objects in proper order
 */
private void recompile(string *names, mapping *leaves, mapping *depend,
		       mapping failed)
{
    int i, j, sz, *indices;
    mixed *objects, obj;
    string name;
    mapping map;

    objects = map_indices(leaves[i]);
    indices = map_values(leaves[i]);
    sz = sizeof(objects);
    for (;;) {
	if (j == sz) {
	    if (++i == sizeof(leaves)) {
		mapping *values;
		int *issues;

		/* get new leaves */
		leaves = ({ });
		indices = map_indices(inherited);
		values = map_values(inherited);
		for (i = sizeof(indices); --i >= 0; ) {
		    map = ([ ]);
		    objects = map_indices(values[i]);
		    issues = map_values(values[i]);
		    for (j = sizeof(objects); --j >= 0; ) {
			if (sizeof(objectd->query_inherited(issues[j])) == 0) {
			    map[objects[j]] = issues[j];
			}
		    }
		    if (map_sizeof(map) != 0) {
			leaves += ({ map });
			inherited[indices[i]] -= map_indices(map);
		    }
		}

		if (sizeof(leaves) == 0) {
		    /* clean up */
		    indices = map_indices(failed);
		    objects = map_values(failed);
		    for (i = map_sizeof(failed); --i >= 0; ) {
			failed[indices[i]] = map_indices(objects[i]);
		    }

		    return;
		}
		i = 0;
	    }
	    j = 0;
	    objects = map_indices(leaves[i]);
	    indices = map_values(leaves[i]);
	    sz = sizeof(objects);
	}

	/* recompile a leaf object */
	obj = objects[j];
	name = (typeof(obj) == T_STRING) ? obj : object_name(obj);
	catch {
	    compile_object(name);
	} : {
	    int index, k;

	    /* recompile failed */
	    index = indices[j] / factor;
	    map = failed[index];
	    if (map) {
		map[name] = 1;
	    } else {
		failed[index] = ([ name : 1 ]);
	    }

	    for (k = sizeof(depend); --k >= 0; ) {
		if ((map=depend[k][index]) && map[obj]) {
		    names[k] = nil;
		}
	    }
	}
	j++;
    }
}

/*
 * NAME:	merge()
 * DESCRIPTION:	merge two mapping structures (trashes first argument)
 */
private mapping merge(mapping m1, mapping m2)
{
    int i, j, v1, v2, sz1, sz2, *indices1, *indices2;
    mapping *values;

    sz1 = map_sizeof(m1);
    sz2 = map_sizeof(m2);
    indices1 = map_indices(m1);
    indices2 = map_indices(m2);
    values = map_values(m2);

    while (i < sz1 && j < sz2) {
	v1 = indices1[i];
	v2 = indices2[j];
	if (v1 < v2) {
	    i++;
	} else if (v1 > v2) {
	    m1[v2] = values[j++];
	} else {
	    m1[v1] += values[j++];
	    i++;
	}
    }
    if (j < sz2) {
	m1 += m2[indices2[j] ..];
    }

    return m1;
}

/*
 * NAME:	upgrade()
 * DESCRIPTION:	upgrade interface function
 */
string upgrade(string creator, string *names, mapping failed)
{
    if (SYSTEM()) {
	rlimits (0; -1) {
	    int i, j, sz, *status;
	    string name;
	    mapping objects, *depend, *leaves;
	    object *list;

	    /* verify write access to objects to be upgraded directly */
	    for (i = 0, sz = sizeof(names); i < sz; i++) {
		if (!access(creator, names[i], WRITE_ACCESS)) {
		    return names[i] + ".c: Access denied.\n";
		}
	    }

	    /*
	     * Gather information about the dependencies.
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
		return "No existing issues.\n";
	    }

	    leaves = map_values(objects);
	    for (i = sizeof(leaves); --i >= 0; ) {
		list = map_indices(leaves[i]);
		for (j = sizeof(list); --j >= 0; ) {
		    name = object_name(list[j]) + ".c";
		    if (!access(creator, name, WRITE_ACCESS)) {
			inherited = nil;
			return name + ": Access denied.\n";
		    }
		    if (!file_info(name)) {
			inherited = nil;
			return name + ": Missing source file.\n";
		    }
		}
	    }
	    if (sizeof(leaves) == 0) {
		leaves = ({ ([ ]) });
	    }

	    objectd->notify_compiling(this_object());

	    /*
	     * Destruct inherited lib objects among the ones that are being
	     * upgraded.
	     */
	    for (i = sz; --i >= 0; ) {
		name = names[i];
		if (sscanf(name, "%*s/lib/") != 0 && (status=status(name)) &&
		    sizeof(objectd->query_inherited(status[O_INDEX])) != 0) {
		    destruct_object(name);
		}
	    }

	    /* Get the show on the road. */
	    recompile(names, leaves, depend, failed);

	    inherited = nil;
	    objectd->notify_compiling(nil);
	}
    }

    return nil;
}


/*
 * NAME:	destruct_lib()
 * DESCRIPTION:	a lib object is about to be destructed
 */
void destruct_lib(string path)
{
    if (previous_object() == objectd) {
	inherited[status(path, O_INDEX) / factor][path] = nil;
    }
}
