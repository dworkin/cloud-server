# include <kernel/kernel.h>
# include <kernel/access.h>
# include <messages.h>
# include <status.h>
# include <type.h>
# include "tls.h"

inherit API_ACCESS;

# define OBJECTSERVER	"/usr/System/sys/objectd"
# define SYSTEMAUTO	"/usr/System/lib/auto"
# define WIZTOOL	"/usr/System/obj/wiztool"


object objectd;			/* object server */
mapping *patching;		/* objects left to patch */
int factor;			/* 2nd level divisor */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize object
 */
static void create()
{
    ::create();

    objectd = find_object(OBJECTSERVER);
    factor = status(ST_ARRAYSIZE);
}


/*
 * NAME:	heirs()
 * DESCRIPTION:	gather information about objects and their heirs
 */
private void heirs(int index, mapping known, mapping libs, mapping leaves,
		   mapping srcmap, mapping deps)
{
    mapping map;
    string path;
    int i, j, **inherited, *issues;

    /* check whether this issue was encountered before */
    map = known[index / factor];
    if (map) {
	if (map[index]) {
	    return;
	}
	map[index] = TRUE;
    } else {
	known[index / factor] = ([ index : TRUE ]);
    }

    path = objectd->query_path(index);
    if (sscanf(path, "%*s/lib/") != 0) {
	inherited = objectd->query_inherited(index);
	for (i = sizeof(inherited); --i >= 0; ) {
	    issues = inherited[i];
	    for (j = sizeof(issues); --j >= 0; ) {
		heirs(issues[j], known, libs, leaves, srcmap, deps);
	    }
	}

	if (status(path, O_INDEX) != index) {
	    return;	/* destructed lib object */
	}
	if (sizeof(objectd->query_inherited(index)) != 0 ||
	    !srcmap[path + ".c"]) {
	    /*
	     * inherited or not explicitly upgraded lib objects
	     */
	    map = libs[index / factor];
	    if (map) {
		map[path] = index;
	    } else {
		libs[index / factor] = ([ path : index ]);
	    }
	    return;
	}
    }

    /*
     * leaf objects are stored in two places
     */
    map = leaves[index / factor];
    if (map) {
	map[path] = index;
    } else {
	leaves[index / factor] = ([ path : index ]);
    }
    map = deps[index / factor];
    if (map) {
	map[path] = index;
    } else {
	deps[index / factor] = ([ path : index ]);
    }
}

/*
 * NAME:	dependencies()
 * DESCRIPTION:	find the dependencies for a list of objects, as library objects,
 *		leaf objects, and leaf dependencies per source file in the list.
 */
private mixed *dependencies(string *sources)
{
    mapping libs, leaves, srcmap, *deps, known;
    int i, j, k, sz, len, **includes, *issues;
    string src;

    libs = ([ ]);
    leaves = ([ ]);
    srcmap = ([ ]);
    sz = sizeof(sources);
    for (i = sz; --i >= 0; ) {
	srcmap[sources[i]] = TRUE;
    }
    deps = allocate(sz);

    for (i = sz; --i >= 0; ) {
	src = sources[i];
	known = ([ ]);
	deps[i] = ([ ]);

	/* include dependencies */
	includes = objectd->query_included(src);
	for (j = sizeof(includes); --j >= 0; ) {
	    issues = includes[j];
	    for (k = sizeof(issues); --k >= 0; ) {
		heirs(issues[k], known, libs, leaves, srcmap, deps[i]);
	    }
	}

	/* object issue dependencies */
	len = strlen(src);
	if (len >= 2 && src[len - 2 ..] == ".c") {
	    issues = objectd->query_issues(src[.. len - 3]);
	    for (j = sizeof(issues); --j >= 0; ) {
		heirs(issues[j], known, libs, leaves, srcmap, deps[i]);
	    }
	}
    }

    return ({ libs, leaves, deps });
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
 * NAME:	compile()
 * DESCRIPTION:	compile one object
 */
private void compile(string name, mapping map, int patch)
{
    string head, tail;
    object obj;

    if (sscanf(name, "%s/@@@/%s", head, tail) != 0) {
	obj = compile_object(name,
			     "inherit \"" + head + "/lib/" + tail + "\";\n");
    } else {
	obj = compile_object(name);
    }

    if (patch) {
	/*
	 * patch object after compilation
	 */
	if (sscanf(name, "%*s/obj/") != 0) {
	    if (sscanf(name, "/kernel/%*s") == 0 &&
		name != "/usr/System/obj/objectd") {
		map[name] = touch_clones(name);
		return;
	    }
	} else if (obj && obj <- "~/lib/auto") {
	    call_touch(obj);
	    return;
	}

	map[name] = nil;
    }
}

/*
 * NAME:	recompile()
 * DESCRIPTION:	recompile leaf objects
 */
private atomic string *recompile(string *sources, mapping *libs,
				 mapping *leaves, mapping *depend, int atom,
				 int patch)
{
    int i, j, sz, *issues;
    string name, err, *objects;
    mapping failed, map;

    if (atom) {
	tls_set(TLS_COMPILE_ERRORS, TRUE);
    }
    tls_set(TLS_UPGRADE_TASK, TRUE);

    /*
     * destruct inherited lib objects
     */
    for (i = sizeof(libs); --i >= 0; ) {
	objects = map_indices(libs[i]);
	for (j = sizeof(objects); --j >= 0; ) {
	    destruct_object(objects[j]);
	}
    }

    failed = ([ ]);
    if (patch) {
	patching = leaves;
    }

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
	    err = catch(compile(name, map, patch));
	    if (err) {
		int index, k;
		object user;

		/* recompile failed */
		failed[tls_get(TLS_COMPILE_FAILED)] = 1;

		/* check which upgraded source files are affected */
		index = issues[j] / factor;
		for (k = sizeof(depend); --k >= 0; ) {
		    map = depend[k][index];
		    if (map && map[name]) {
			sources[k] = nil;
		    }
		}

		if (atom) {
		    add_atomic_message(err);
		} else if ((user=this_user())) {
		    user->message(MSG_FORMATTED, err + "\n");
		}
	    }
	}
    }

    if (atom) {
	tls_set(TLS_COMPILE_ERRORS, nil);
    }

    objects = map_indices(failed);
    if (sizeof(objects) != 0 && atom) {
	/*
	 * some objects could not be compiled -- fail atomically
	 */
	add_atomic_message("#" + implode(objects, "#"));
	error("Upgrade failed");
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
	    mapping inherited, objects, *depend, *leaves;

	    if (tls_get(TLS_UPGRADE_TASK)) {
		return "An upgrade was already started.";
	    }
	    if (patch && patching) {
		return "Still patching after previous upgrade.";
	    }

	    /* verify write access to objects to be upgraded directly */
	    for (i = 0, sz = sizeof(names); i < sz; i++) {
		if (!access(owner, names[i], WRITE_ACCESS)) {
		    return names[i] + ": Access denied.";
		}
	    }

	    /*
	     * gather information about the dependencies
	     */
	    ({ inherited, objects, depend }) = dependencies(names);
	    if (map_sizeof(inherited) + map_sizeof(objects) == 0) {
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
			return str + ": Access denied.";
		    }
		    if (!file_info(str) && sscanf(str, "%*s/@@@/") == 0) {
			return str + ": Missing source file.";
		    }
		}
	    }

	    /*
	     * recompile leaf objects
	     */
	    catch {
		return recompile(names, map_values(inherited), leaves, depend,
				 atom, patch);
	    } : {
		object user;

		list = get_atomic_messages();
		sz = sizeof(list);
		if (sz != 0) {
		    str = list[sz - 1];
		    if (str[0] == '#') {
			--sz;
		    }
		    user = this_user();
		    if (user) {
			for (i = 0; i < sz; i++) {
			    user->message(MSG_FORMATTED, list[i] + "\n");
			}
		    }
		    if (str[0] == '#') {
			return explode(str, "#");
		    }
		}

		return "Upgrade failed.";
	    }
	}
    }
}


/*
 * NAME:	generate_leaf()
 * DESCRIPTION:	generate a leaf object
 */
string generate_leaf(string path)
{
    if (previous_program() == SYSTEMAUTO) {
	string str, tail;

	str = object_name(previous_object());
	if (sscanf(str, WIZTOOL + "#%*d") == 0 &&
	    !access(str, path, READ_ACCESS)) {
	    error("Access denied");
	}
	sscanf(path, "%s/lib/%s", str, tail);
	str += "/@@@/" + tail;
	if (!find_object(str)) {
	    compile_object(str, "inherit \"" + path + "\";\n");
	}
	return str;
    }
}
