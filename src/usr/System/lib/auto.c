# include <kernel/kernel.h>
# include <status.h>
# include <type.h>
# include <Continuation.h>
# include "tls.h"

# define SYSTEM_AUTO		"/usr/System/lib/auto"
# define OBJECT_SERVER		"/usr/System/sys/objectd"
# define UPGRADE_SERVER		"/usr/System/sys/upgraded"
# define CONTINUATION		"/lib/Continuation"
# define CHAINED_CONTINUATION	"/lib/ChainedContinuation"
# define DELAYED_CONTINUATION	"/lib/DelayedContinuation"
# define ITERATIVE_CONTINUATION	"/lib/IterativeContinuation"
# define DIST_CONTINUATION	"/lib/DistContinuation"


private mapping storage;	/* uninitialized until used */

/*
 * NAME:	_F_init()
 * DESCRIPTION:	System level creator function
 */
static nomask int _F_init()
{
    if (previous_program() == AUTO) {
	mixed *args;

	args = ::tls_get(TLS_ARGUMENTS);
	if (args) {
	    ::tls_set(TLS_ARGUMENTS, nil);
	    create(args...);
	} else {
	    call_limited("create");
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
	path = DRIVER->normalize_path(path);
	if (sscanf(path, "%*s/obj/") == 0) {
	    error("Invalid path");
	}
    }
    ::tls_set(TLS_ARGUMENTS, args);
    return ::clone_object(path);
}

/*
 * NAME:	_F_copy()
 * DESCRIPTION:	call copy() in a newly copied object
 */
nomask void _F_copy()
{
    if (previous_program() == SYSTEM_AUTO) {
	this_object()->copy();
    }
}

/*
 * NAME:	new_object()
 * DESCRIPTION:	create and initialize a new light-weight object
 */
static object new_object(string path, mixed args...)
{
    string uid;

    if (path) {
	path = DRIVER->normalize_path(path);
	switch (path) {
	case CONTINUATION:
	case CHAINED_CONTINUATION:
	case DELAYED_CONTINUATION:
	case ITERATIVE_CONTINUATION:
	case DIST_CONTINUATION:
	    uid = "System";
	    break;

	default:
	    if (sscanf(path, "%*s/obj/") != 0 || sscanf(path, "%*s/sys/") != 0)
	    {
		error("Invalid path");
	    }
	    break;
	}
	if (sscanf(path, "%*s/lib/") != 0 && status(path, O_INDEX) != nil) {
	    /* let upgrade server generate a leaf object */
	    path = UPGRADE_SERVER->generate_leaf(path);
	}
    }
    ::tls_set(TLS_ARGUMENTS, args);
    return ::new_object(path, uid);
}

/*
 * NAME:	copy_object()
 * DESCRIPTION:	copy this light-weight object
 */
static object copy_object()
{
    object obj;

    if (sscanf(object_name(this_object()), "%*s#-1") == 0) {
	error("Not a light-weight object");
    }
    obj = ::new_object(this_object());
    obj->_F_copy();

    return obj;
}

/*
 * NAME:	find_object()
 * DESCRIPTION:	prevent finding clone masters and generated leaf objects by name
 */
static object find_object(string path)
{
    object obj;

    obj = ::find_object(path);
    if (obj) {
	path = object_name(obj);
	if (sscanf(path, "%*s/obj/%*s#") == 1 ||
	    sscanf(path, "%*s/@@@/%*s#") == 1) {
	    return nil;
	}
    }

    return obj;
}

/*
 * NAME:	compile_object()
 * DESCRIPTION:	compile an object
 */
static object compile_object(string path, string source...)
{
    object obj;

    if (path) {
	path = DRIVER->normalize_path(path);
	if (sscanf(path, "%*s/@@@/") != 0) {
	    error("Cannot compile leaf object");
	}
	if (sscanf(path, "%*s/lib/") + sscanf(path, "%*s/obj/") +
	    sscanf(path, "%*s/sys/") > 1) {
	    error("Ambiguous object");
	}
    }
    obj = ::compile_object(path, source...);
    if (obj) {
	if (sscanf(path, "%*s/obj/") != 0) {
	    return nil;
	} else if (sscanf(path, "%*s/sys/") != 0) {
	    call_other(obj, "???");
	}
    }
    return obj;
}


/*
 * NAME:	tls_set()
 * DESCRIPTION:	set Task Local Storage value
 */
static void tls_set(string index, mixed value)
{
    string program;

    if (sscanf(index, "%s::%s", program, index) != 0) {
	if (program != "") {
	    error("Illegal use of tls_set() for private variable");
	} else {
	    index = previous_program() + "::" + index;
	}
    }
    ::tls_set(index, value);
}

/*
 * NAME:	tls_get()
 * DESCRIPTION:	get Task Local Storage value
 */
static mixed tls_get(string index)
{
    string program;

    if (sscanf(index, "%s::%s", program, index) != 0) {
	if (program != "") {
	    error("Illegal use of tls_get() for private variable");
	} else {
	    index = previous_program() + "::" + index;
	}
    }
    return ::tls_get(index);
}


/*
 * NAME:	_F_touch()
 * DESCRIPTION:	touch call gate
 */
nomask int _F_touch()
{
    if (previous_program() == OBJECT_SERVER) {
	this_object()->patch();
    }
    return FALSE;
}


# define REF_CONT	0	/* continuation */
# define REF_COUNT	1	/* callback countdown */
# define REF_TIMEOUT	2	/* timeout handle */

/*
 * NAME:	startContinuation()
 * DESCRIPTION:	runNext a continuation, start first callout if none running yet
 */
static void startContinuation(mixed *continuation, int parallel)
{
    if (previous_program() == CONTINUATION) {
	mixed *ref;

	if (parallel || !(ref=::tls_get(TLS_CONTINUATION))) {
	    /*
	     * schedule first continuation
	     */
	    ref = ({ ({ }), 0, 0 });
	    if (!parallel) {
		::tls_set(TLS_CONTINUATION, ref);
	    }
	    ::call_out_other(continuation[CONT_ORIGIN], "_F_continued", 0, ref);
	}

	/*
	 * run these continuations before all others
	 */
	ref[REF_CONT] = continuation + ref[REF_CONT];
    }
}

/*
 * NAME:	suspendContinuation()
 * DESCRIPTION:	suspend current continuation
 */
static object suspendContinuation()
{
    mixed *ref, *continued;
    object continuation;

    ref = ::tls_get(TLS_CONTINUATION);
    if (!ref || sizeof(continued=ref[REF_CONT]) == 0 || !continued[CONT_ORIGIN])
    {
	error("No continuation");
    }
    ref[REF_CONT] = ({ });

    continuation = new Continuation;
    continuation->saveContinuation(continued);
    return continuation;
}

/*
 * NAME:	continued()
 * DESCRIPTION:	run a continuation
 */
private void continued(mixed *ref)
{
    mixed *continued;
    int type, token, sz, i;
    mixed val, objs, delay, args;
    object origin;
    string func;

    continued = ref[REF_CONT];
    if (sizeof(continued) == 0) {
	return;
    }

    ({ objs, delay, origin, func, args, val }) = continued[.. CONT_SIZE - 1];
    if (origin != this_object()) {
	::call_out_other(origin, "_F_continued", 0, ref);
	return;
    }
    ::tls_set(TLS_CONTINUATION, ref);

    switch (typeof(objs)) {
    case T_INT:
	ref[REF_CONT] = continued[CONT_SIZE ..];
	if (objs) {
	    /* chained */
	    val = call_other(this_object(), func, (args + ({ val }))...);
	} else {
	    /* standard */
	    val = call_other(this_object(), func, args...);
	}
	break;

    case T_OBJECT:
	/* iterator */
	val = objs->next();
	if (val != nil) {
	    call_other(this_object(), func, val, args...);
	}
	break;

    case T_ARRAY:
	/*
	 * distributed continuation
	 */
	sz = sizeof(objs);
	ref[REF_CONT] = continued = continued[CONT_SIZE ..];
	if (!storage) {
	    token = 0;
	    storage = ([ "token" : 0, 0 : ref ]);
	} else {
	    for (token = storage["token"]; storage[++token]; ) ;
	    storage["token"] = token;
	    storage[token] = ref;
	}
	ref[REF_COUNT] = sz;
	ref[REF_TIMEOUT] = ::call_out_other(this_object(),
					    "_F_timeoutContinuation", delay,
					    token);

	if (sizeof(continued) != 0 && typeof(continued[CONT_OBJS]) == T_INT &&
	    continued[CONT_OBJS]) {
	    /* return values from distributed continuations */
	    continued[CONT_VAL] = allocate(sz);
	}
	for (i = 0; i < sz; i++) {
	    ::call_out_other(objs[i], "_F_continued", 0, ({
		({
		    0, 0, objs[i], func, args, nil,		/* extern */
		    this_object(), 0, objs[i], nil, ({ token, i }), nil
								/* callback */
		}),
		0,
		0
	    }));
	}
	return;
    }

    continued = ref[REF_CONT];
    while (sizeof(continued) != 0) {
	objs = continued[CONT_OBJS];
	switch (typeof(objs)) {
	case T_NIL:
	    /* callback to destructed object */
	    return;

	case T_INT:
	    if (objs) {
		/* return value needed by next step */
		continued[CONT_VAL] = val;
	    }
	    break;

	case T_OBJECT:
	    if (continued[CONT_FUNC]) {
		/* iterator */
		if (objs->end()) {
		    ref[REF_CONT] = continued = continued[CONT_SIZE ..];
		    continue;
		}
	    } else {
		/* callback */
		::call_out_other(objs, "_F_doneContinuation", 0, val,
				 continued[CONT_ARGS]...);
		return;
	    }
	    break;
	}
	::call_out_other(continued[CONT_ORIGIN], "_F_continued", delay, ref);
	break;
    }
}

/*
 * NAME:	_F_return()
 * DESCRIPTION:	return argument
 */
nomask mixed _F_return(mixed arg)
{
    return arg;
}

/*
 * NAME:	_F_continued()
 * DESCRIPTION	run continuation from callout
 */
nomask void _F_continued(mixed *ref)
{
    if (previous_program() == AUTO) {
	continued(ref);
    }
}

/*
 * NAME:	_F_doneContinuation()
 * DESCRIPTION:	finished performing a distributed continuation
 */
nomask void _F_doneContinuation(mixed result, int token, int index)
{
    if (previous_program() == AUTO && storage) {
	mixed *ref, *continued;

	ref = storage[token];
	if (ref) {
	    continued = ref[REF_CONT];
	    if (sizeof(continued) != 0 &&
		typeof(continued[CONT_VAL]) == T_ARRAY) {
		continued[CONT_VAL][index] = result;
	    }
	    if (--ref[REF_COUNT] == 0) {
		storage[token] = nil;
		::remove_call_out(ref[REF_TIMEOUT]);
		continued(ref);
	    }
	}
    }
}

/*
 * NAME:	_F_timeoutContinuation()
 * DESCRIPTION:	a distributed continuation timed out
 */
nomask void _F_timeoutContinuation(int token)
{
    if (previous_program() == AUTO && storage) {
	mixed *ref;

	ref = storage[token];
	if (ref) {
	    storage[token] = nil;
	    continued(ref);
	}
    }
}

/*
 * NAME:	call_out()
 * DESCRIPTION: prevent System auto functions from being called by callout
 */
static int call_out(string func, mixed args...)
{
    if (function_object(func, this_object()) == SYSTEM_AUTO) {
	error("Illegal callout");
    }
    return ::call_out(func, args...);
}

/*
 * NAME:	call_out_other()
 * DESCRIPTION:	prevent System auto functions from being called by callout
 */
static int call_out_other(object obj, string func, mixed args...)
{
    if (function_object(func, obj) == SYSTEM_AUTO) {
	error("Illegal callout");
    }
    return ::call_out_other(obj, func, args...);
}

/*
 * Disabled functions.
 */
static string query_ip_number(object obj)	{ return nil; }
static string query_ip_name(object obj)		{ return nil; }
