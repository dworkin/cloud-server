# include <kernel/kernel.h>
# include <kernel/rsrc.h>
# include <kernel/access.h>
# include <kernel/user.h>
# include <status.h>
# include <type.h>
# include <trace.h>


# define TLSVAR2		::call_trace()[1][TRACE_FIRSTARG][1]
# define TLSVAR4		::call_trace()[1][TRACE_FIRSTARG][3]
# define LONG_TIME		(365 * 24 * 60 * 60)
# define CHECKARG(arg, n, func)	if (!(arg)) badarg((n), (func))

/*
 * NAME:	badarg()
 * DESCRIPTION:	called when an argument check failed
 */
private void badarg(int n, string func)
{
    error("Bad argument " + n + " for function " + func);
}


private string creator, owner;	/* creator and owner of this object */
private mapping resources;	/* resources associated with this object */

/*
 * NAME:	query_owner()
 * DESCRIPTION:	query the owner of an object
 */
nomask string query_owner()
{
    return owner;
}

/*
 * NAME:	_F_rsrc_incr()
 * DESCRIPTION:	increase/decrease a resource associated with this object
 */
nomask void _F_rsrc_incr(string rsrc, int incr)
{
    if (previous_program() == RSRCOBJ) {
	if (!resources) {
	    resources = ([ rsrc : incr ]);
	} else if (!resources[rsrc]) {
	    resources[rsrc] = incr;
	} else if (!(resources[rsrc] += incr)) {
	    resources[rsrc] = nil;
	}
    }
}

void create(varargs int clone) { }	/* default high-level create function */

/*
 * NAME:	_F_create()
 * DESCRIPTION:	kernel creator function
 */
nomask void _F_create()
{
    if (!creator) {
	string oname;
# ifdef CREATOR
	string cname;
# endif
	object driver;
	int clone;

	rlimits (-1; -1) {
	    /*
	     * set creator and owner
	     */
	    oname = object_name(this_object());
	    driver = ::find_object(DRIVER);
	    creator = driver->creator(oname);
	    if (sscanf(oname, "%s#%d", oname, clone) != 0) {
		owner = TLSVAR2;
		clone = (clone >= 0);
		if (clone && oname != BINARY_CONN && oname != TELNET_CONN) {
		    /*
		     * register object
		     */
		    driver->clone(this_object(), owner);
		}
	    } else {
		owner = creator;
	    }
	    if (!creator) {
		creator = "";
	    }

# ifdef CREATOR
	    cname = function_object(CREATOR, this_object());
	    if (cname && sscanf(cname, USR_DIR + "/System/%*s") != 0) {
		/* extra initialisation function */
		if (call_other(this_object(), CREATOR, clone)) {
		    return;
		}
	    }
# endif
	}
	/* call higher-level creator function */
	if (sscanf(oname, "%*s" + CLONABLE_SUBDIR) == 0 &&
	    sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
	    create();
	} else {
	    create(clone);
	}
    }
}

/*
 * NAME:	_F_destruct()
 * DESCRIPTION:	prepare object for being destructed
 */
nomask void _F_destruct()
{
    if (previous_program() == AUTO && resources) {
	object rsrcd;
	string *names;
	int *values;
	int i;

	/*
	 * decrease resources associated with object
	 */
	rsrcd = ::find_object(RSRCD);
	names = map_indices(resources);
	values = map_values(resources);
	i = sizeof(names);
	while (--i >= 0) {
	    rsrcd->rsrc_incr(owner, names[i], this_object(), -values[i]);
	}
    }
}


/*
 * NAME:	find_object()
 * DESCRIPTION:	find an object
 */
static object find_object(string path)
{
    CHECKARG(path, 1, "find_object");
    if (!this_object()) {
	return nil;
    }

    path = ::find_object(DRIVER)->normalize_path(path,
						 object_name(this_object()) +
						 "/..",
						 creator);
    if (sscanf(path, "%*s" + INHERITABLE_SUBDIR) != 0 ||
	sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s#") == 1 ||
	sscanf(path, "%*s" + LIGHTWEIGHT_SUBDIR) != 0) {
	/*
	 * It is not possible to find a class object by name, or to call a
	 * function in it.
	 */
	return nil;
    }
    return ::find_object(path);
}

/*
 * NAME:	destruct_object()
 * DESCRIPTION:	destruct an object
 */
static int destruct_object(mixed obj)
{
    object driver;
    string oname, oowner;
    int clone, lib, class;

    /* check and translate argument */
    driver = ::find_object(DRIVER);
    if (typeof(obj) == T_STRING) {
	if (!this_object()) {
	    return FALSE;
	}
	obj = ::find_object(driver->normalize_path(obj,
						   object_name(this_object()) +
						   "/..",
						   creator));
	if (!obj) {
	    return FALSE;
	}
    } else {
	CHECKARG(typeof(obj) == T_OBJECT, 1, "destruct_object");
	if (!this_object()) {
	    return FALSE;
	}
    }

    /*
     * check privileges
     */
    oname = object_name(obj);
    clone = sscanf(oname, "%s#%d", oname, lib);
    if (clone && lib < 0) {
	error("Cannot destruct non-persistent object");
    }
    lib = sscanf(oname, "%*s" + INHERITABLE_SUBDIR);
    class = (!clone &&
	     (lib || sscanf(oname, "%*s" + CLONABLE_SUBDIR) != 0 ||
	      sscanf(oname, "%*s" + LIGHTWEIGHT_SUBDIR) != 0));
    oowner = (class) ? driver->creator(oname) : obj->query_owner();
    if ((sscanf(oname, "/kernel/%*s") != 0 && !lib && !KERNEL()) ||
	(creator != "System" && owner != oowner)) {
	error("Cannot destruct object: not owner");
    }

    rlimits (-1; -1) {
	if (oname != BINARY_CONN && oname != TELNET_CONN) {
	    driver->destruct(object_name(obj), oowner);
	    if (clone) {
		/*
		 * non-clones are handled by driver->remove_program()
		 */
		::find_object(RSRCD)->rsrc_incr(owner, "objects", nil, -1);
	    }
	}
	if (!class) {
	    obj->_F_destruct();
	}
	::destruct_object(obj);
    }
    return TRUE;
}

/*
 * NAME:	compile_object()
 * DESCRIPTION:	compile a master object
 */
static object compile_object(string path, string source...)
{
    string oname, uid;
    object driver, rsrcd, obj;
    int *rsrc, class, kernel, new, stack, ticks;

    CHECKARG(path, 1, "compile_object");
    if (!this_object()) {
	error("Permission denied");
    }

    /*
     * check access
     */
    oname = object_name(this_object());
    driver = ::find_object(DRIVER);
    path = driver->normalize_path(path, oname + "/..", creator);
    class = sscanf(path, "%*s" + INHERITABLE_SUBDIR);
    kernel = sscanf(path, "/kernel/%*s");
    uid = driver->creator(path);
    if ((sizeof(source) != 0 && kernel) ||
	(creator != "System" &&
	 !::find_object(ACCESSD)->access(oname, path,
					 ((class || !uid) &&
					  sizeof(source) == 0 && !kernel) ?
					  READ_ACCESS : WRITE_ACCESS))) {
	error("Access denied");
    }
    class += sscanf(path, "%*s" + CLONABLE_SUBDIR) +
	     sscanf(path, "%*s" + LIGHTWEIGHT_SUBDIR);
    if (class > 1) {
	error("Ambiguous object");
    }

    /*
     * check resource usage
     */
    rsrcd = ::find_object(RSRCD);
    rsrc = rsrcd->rsrc_get(uid, "objects");
    if (rsrc[RSRC_USAGE] >= rsrc[RSRC_MAX] && rsrc[RSRC_MAX] >= 0) {
	error("Too many objects");
    }

    /*
     * do the compiling
     */
    new = !::find_object(path);
    stack = ::status()[ST_STACKDEPTH];
    ticks = ::status()[ST_TICKS];
    rlimits (-1; -1) {
	catch {
	    if (new && !class) {
		if ((stack >= 0 &&
		     stack - 2 < rsrcd->rsrc_get(uid,
						 "create stack")[RSRC_MAX]) ||
		    (ticks >= 0 &&
		     ticks < rsrcd->rsrc_get(uid, "create ticks")[RSRC_MAX])) {
		    error("Insufficient stack or ticks to create object");
		}
	    }
	    driver->compiling(path);
	    if (sizeof(source) != 0) {
		obj = ::compile_object(path, source...);
	    } else {
		obj = ::compile_object(path);
	    }
	    if (new) {
		rsrcd->rsrc_incr(uid, "objects", nil, 1);
	    }
	    driver->compile(path, uid, source...);
	} : {
	    driver->compile_failed(path, uid);
	    rlimits (stack; ticks) {
		error(TLSVAR2);
	    }
	}
    }
    if (new && !class) {
	call_other(obj, "???");		/* initialize */
    }

    return (class) ? nil : obj;
}

/*
 * NAME:	clone_object()
 * DESCRIPTION:	clone an object
 */
static object clone_object(string path, varargs string uid)
{
    string oname;
    object rsrcd, obj;
    int *rsrc, stack, ticks;

    CHECKARG(path, 1, "clone_object");
    if (uid) {
	CHECKARG(creator == "System", 1, "clone_object");
    } else {
	uid = owner;
    }
    if (!this_object()) {
	error("Permission denied");
    }

    /*
     * check access
     */
    oname = object_name(this_object());
    path = ::find_object(DRIVER)->normalize_path(path, oname + "/..", creator);
    if ((sscanf(path, "/kernel/%*s") != 0 && !KERNEL()) ||
	(creator != "System" &&
	 !::find_object(ACCESSD)->access(oname, path, READ_ACCESS))) {
	/*
	 * kernel objects can only be cloned by kernel objects, and cloning
	 * in general requires read access
	 */
	error("Access denied");
    }

    /*
     * check if object can be cloned
     */
    if (!owner || !(obj=::find_object(path)) ||
	sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s#") != 1) {
	/*
	 * no owner for clone, master object not compiled, or not path of
	 * clonable
	 */
	error("Cannot clone " + path);
    }

    /*
     * check resource usage
     */
    rsrcd = ::find_object(RSRCD);
    if (path != BINARY_CONN && path != TELNET_CONN && path != RSRCOBJ) {
	rsrc = rsrcd->rsrc_get(uid, "objects");
	if (rsrc[RSRC_USAGE] >= rsrc[RSRC_MAX] && rsrc[RSRC_MAX] >= 0) {
	    error("Too many objects");
	}
    }
    if (::status()[ST_NOBJECTS] == ::status()[ST_OTABSIZE]) {
	error("Too many objects");
    }

    /*
     * do the cloning
     */
    stack = ::status()[ST_STACKDEPTH];
    ticks = ::status()[ST_TICKS];
    catch {
	rlimits (-1; -1) {
	    if ((stack >= 0 &&
		 stack - 2 < rsrcd->rsrc_get(uid, "create stack")[RSRC_MAX]) ||
		(ticks >= 0 &&
		 ticks < rsrcd->rsrc_get(uid, "create ticks")[RSRC_MAX])) {
		error("Insufficient stack or ticks to create object");
	    }
	    if (path != BINARY_CONN && path != TELNET_CONN && path != RSRCOBJ) {
		rsrcd->rsrc_incr(uid, "objects", nil, 1);
	    }
	    TLSVAR2 = uid;
	}
    } : error(TLSVAR2);
    return ::clone_object(obj);
}

/*
 * NAME:	new_object()
 * DESCRIPTION:	create a new non-persistent object
 */
static object new_object(mixed obj, varargs string uid)
{
    string oname, str;
    object rsrcd;
    int new, stack, ticks;

    if (!this_object()) {
	error("Permission denied");
    }
    switch (typeof(obj)) {
    case T_STRING:
	oname = object_name(this_object());
	str = ::find_object(DRIVER)->normalize_path(obj, oname + "/..",
			    creator);
	obj = ::find_object(str);
	new = TRUE;
	break;

    case T_OBJECT:
	str = object_name(obj);
	if (sscanf(str, "%*s#-1") == 0) {
	    error("new_object() requires non-persistent object argument");
	}
	new = FALSE;
	break;

    default:
	error("Bad argument 1 for function new_object");
    }
    if (uid) {
	CHECKARG(new && creator == "System", 1, "new_object");
    } else {
	uid = owner;
    }

    /*
     * create the object
     */
    if (new) {
	/*
	 * check access
	 */
	if ((creator != "System" &&
	     !::find_object(ACCESSD)->access(oname, str, READ_ACCESS))) {
	    error("Access denied");
	}

	/*
	 * check if object can be created
	 */
	if (!obj || sscanf(str, "%*s" + LIGHTWEIGHT_SUBDIR) == 0) {
	    /*
	     * master object not compiled, or not path of non-persistent object
	     */
	    error("Cannot create new instance of " + str);
	}

	rsrcd = ::find_object(RSRCD);
	stack = ::status()[ST_STACKDEPTH];
	ticks = ::status()[ST_TICKS];
	catch {
	    rlimits (-1; -1) {
		if ((stack >= 0 &&
		     stack - 2 < rsrcd->rsrc_get(uid,
						 "create stack")[RSRC_MAX]) ||
		    (ticks >= 0 &&
		     ticks < rsrcd->rsrc_get(uid, "create ticks")[RSRC_MAX])) {
		    error("Insufficient stack or ticks to create object");
		}
		TLSVAR2 = uid;
	    }
	} : error(TLSVAR2);
    }
    return ::new_object(obj);
}

/*
 * NAME:	process_trace()
 * DESCRIPTION:	filter out function call arguments from a call trace
 */
private mixed *process_trace(object driver, mixed *trace)
{
    if (sizeof(trace) > TRACE_FIRSTARG &&
	creator != driver->creator(trace[TRACE_PROGNAME])) {
	/* remove arguments */
	return trace[.. TRACE_FIRSTARG - 1];
    }
    return trace;
}

/*
 * NAME:	call_trace()
 * DESCRIPTION:	function call trace
 */
static mixed *call_trace(varargs mixed index)
{
    mixed *trace;

    if (index == nil) {
	trace = ::call_trace();
	if (previous_program() != RSRCOBJ) {
	    trace[1][TRACE_FIRSTARG] = nil;
	}
	if (creator != "System") {
	    object driver;
	    int i;

	    driver = ::find_object(DRIVER);
	    for (i = sizeof(trace) - 1; --i >= 0; ) {
		trace[i] = process_trace(driver, trace[i]);
	    }
	}
    } else {
	trace = ::call_trace()[index];
	if (index == 1 && previous_program() != RSRCOBJ) {
	    trace[TRACE_FIRSTARG] = nil;
	}
	if (creator != "System") {
	    trace = process_trace(::find_object(DRIVER), trace);
	}
    }

    return trace;
}

/*
 * NAME:	process_precompiled()
 * DESCRIPTION:	process precompiled objects in a status() return value
 */
private void process_precompiled(mixed *precompiled)
{
    int i;

    if (precompiled) {
	for (i = sizeof(precompiled); --i >= 0; ) {
	    precompiled[i] = object_name(precompiled[i]);
	}
    }
}

/*
 * NAME:	process_callouts()
 * DESCRIPTION:	process callouts in a status(obj) return value
 */
private mixed **process_callouts(object obj, mixed **callouts)
{
    int i;
    string oname;
    mixed *co;

    if (callouts && (i=sizeof(callouts)) != 0) {
	oname = object_name(obj);
	if (sscanf(oname, "/kernel/%*s") != 0) {
	    /* can't see callouts in kernel objects */
	    return ({ });
	} else if (obj != this_object() && creator != "System" &&
		   (!owner || owner != obj->query_owner())) {
	    /* remove arguments from callouts */
	    do {
		--i;
		co = callouts[i];
		callouts[i] = ({ co[CO_HANDLE], co[CO_FIRSTXARG],
				 co[CO_DELAY] });
	    } while (i != 0);
	} else {
	    do {
		--i;
		co = callouts[i];
		callouts[i] = ({ co[CO_HANDLE], co[CO_FIRSTXARG],
				 co[CO_DELAY] }) +
			      co[CO_FIRSTXARG + 1];
	    } while (i != 0);
	}
    }

    return callouts;
}

/*
 * NAME:	status()
 * DESCRIPTION:	get information about an object
 */
static mixed status(varargs mixed obj, mixed index)
{
    mixed status;
    object driver;

    if (!this_object()) {
	return nil;
    }

    switch (typeof(obj)) {
    case T_NIL:
	CHECKARG(index == nil, 1, "status");
	status = ::status();
	if (status[ST_STACKDEPTH] >= 0) {
	    status[ST_STACKDEPTH]++;
	}
	process_precompiled(status[ST_PRECOMPILED]);
	break;

    case T_INT:
	if (obj == -1) {
	    return (index == nil) ? ::status(1) : ::status(1)[index];
	} else {
	    CHECKARG(index == nil, 1, "status");
	    status = ::status()[obj];
	    if (obj == ST_STACKDEPTH && status >= 0) {
		status++;
	    } else if (obj == ST_PRECOMPILED) {
		process_precompiled(status);
	    }
	}
	break;

    case T_STRING:
	/* get corresponding object */
	driver = ::find_object(DRIVER);
	obj = ::find_object(driver->normalize_path(obj,
						   object_name(this_object()) +
						   "/..",
						   creator));
	if (!obj) {
	    return nil;
	}
	/* fall through */
    case T_OBJECT:
	switch (typeof(index)) {
	case T_NIL:
	    status = ::status(obj);
	    status[O_CALLOUTS] = process_callouts(obj, status[O_CALLOUTS]);
	    break;

	case T_INT:
	    status = ::status(obj)[index];
	    if (index == O_CALLOUTS) {
		status = process_callouts(obj, status);
	    }
	    break;

	default:
	    badarg(2, "status");
	}
	break;
    }

    return status;
}

/*
 * NAME:	this_user()
 * DESCRIPTION:	return the user object and not a connection object
 */
static object this_user()
{
    object user;

    user = ::this_user();
    if (!user) {
	user = TLSVAR4;
    }
    while (user && user <- LIB_CONN) {
	user = user->query_user();
    }
    return user;
}

/*
 * NAME:	users()
 * DESCRIPTION:	return an array with the current user objects
 */
static object *users()
{
    if (!this_object()) {
	return nil;
    } else if (object_name(this_object()) == USERD) {
	/* connection objects */
	return ::users();
    } else {
	return ::find_object(USERD)->query_users();
    }
}

/*
 * NAME:	connect()
 * DESCRIPTION:	establish an outbound connection
 */
static void connect(string address, int port)
{
    if (!(this_object() <- LIB_CONN)) {
	error("Permission denied");
    }
    ::connect(address, port);
}

/*
 * NAME:	swapout()
 * DESCRIPTION:	swap out all objects
 */
static void swapout()
{
    if (creator != "System") {
	error("Permission denied");
    }
    ::swapout();
}

/*
 * NAME:	dump_state()
 * DESCRIPTION:	create snapshot
 */
static void dump_state(varargs int incr)
{
    if (creator != "System" || !this_object()) {
	error("Permission denied");
    }
    rlimits (-1; -1) {
	::find_object(DRIVER)->prepare_reboot();
	::dump_state(incr);
    }
}

/*
 * NAME:	shutdown()
 * DESCRIPTION:	shutdown the system
 */
static void shutdown(varargs int hotboot)
{
    if (creator != "System" || !this_object()) {
	error("Permission denied");
    }
    rlimits (-1; -1) {
	::shutdown(hotboot);
	::find_object(DRIVER)->message("System halted.\n");
    }
}

/*
 * NAME:	call_touch()
 * DESCRIPTION:	arrange to be warned when a function is called in an object
 */
static void call_touch(object obj)
{
    if (creator != "System") {
	error("Permission denied");
    }
    ::call_touch(obj);
}


/*
 * NAME:	_F_call_limited()
 * DESCRIPTION:	call a function with limited stack depth and ticks
 */
private mixed _F_call_limited(mixed arg1, mixed *args)
{
    object rsrcd;
    int stack, ticks;
    string func;
    mixed tls, *limits, result;

    rsrcd = ::find_object(RSRCD);
    func = arg1;
    stack = ::status()[ST_STACKDEPTH];
    ticks = ::status()[ST_TICKS];
    rlimits (-1; -1) {
	tls = ::call_trace()[1][TRACE_FIRSTARG];
	if (tls == arg1) {
	    tls = arg1 = allocate(::find_object(DRIVER)->query_tls_size());
	}
	limits = tls[0] = rsrcd->call_limits(tls[0], owner, stack, ticks);
    }

    rlimits (limits[LIM_MAXSTACK]; limits[LIM_MAXTICKS]) {
	result = call_other(this_object(), func, args...);

	ticks = ::status()[ST_TICKS];
	rlimits (-1; -1) {
	    rsrcd->update_ticks(limits, ticks);
	    tls[0] = limits[LIM_NEXT];

	    return result;
	}
    }
}

/*
 * NAME:	call_limited()
 * DESCRIPTION:	call a function with the current object owner's resource limits
 */
static mixed call_limited(string func, mixed args...)
{
    CHECKARG(func, 1, "call_limited");
    if (!this_object()) {
	return nil;
    }
    CHECKARG(function_object(func, this_object()) != AUTO || func == "create",
	     1, "call_limited");

    return _F_call_limited(func, args);
}

/*
 * NAME:	call_out()
 * DESCRIPTION:	start a callout
 */
static int call_out(string func, mixed delay, mixed args...)
{
    int type;
    string oname;

    CHECKARG(func, 1, "call_out");
    type = typeof(delay);
    CHECKARG(type == T_INT || type == T_FLOAT, 2, "call_out");
    if (!this_object()) {
	return 0;
    }
    CHECKARG(function_object(func, this_object()) != AUTO || func == "create",
	     1, "call_out");
    oname = object_name(this_object());
    if (sscanf(oname, "%*s#-1") != 0) {
	error("Callout in non-persistent object");
    }

    /*
     * add callout
     */
    if (sscanf(oname, "/kernel/%*s") != 0) {
	/* direct callouts for kernel objects */
	return ::call_out(func, delay, args...);
    }
    return ::call_out("_F_callout", delay, func, args);
}

/*
 * NAME:	_F_callout()
 * DESCRIPTION:	callout gate
 */
nomask void _F_callout(string func, mixed *args)
{
    if (!previous_program()) {
	_F_call_limited(func, args);
    }
}


/*
 * NAME:	read_file()
 * DESCRIPTION:	read a string from a file
 */
static string read_file(string path, varargs int offset, int size)
{
    string oname;

    CHECKARG(path, 1, "read_file");
    if (!this_object()) {
	error("Permission denied");
    }

    oname = object_name(this_object());
    path = ::find_object(DRIVER)->normalize_path(path, oname + "/..", creator);
    if (creator != "System" &&
	!::find_object(ACCESSD)->access(oname, path, READ_ACCESS)) {
	error("Access denied");
    }

    return ::read_file(path, offset, size);
}

/*
 * NAME:	write_file()
 * DESCRIPTION:	write a string to a file
 */
static int write_file(string path, string str, varargs int offset)
{
    string oname, fcreator;
    object driver, rsrcd;
    int size, result, *rsrc;

    CHECKARG(path, 1, "write_file");
    CHECKARG(str, 2, "write_file");
    if (!this_object()) {
	error("Permission denied");
    }

    oname = object_name(this_object());
    driver = ::find_object(DRIVER);
    path = driver->normalize_path(path, oname + "/..", creator);
    if (sscanf(path, "/kernel/%*s") != 0 ||
	sscanf(path, "/include/kernel/%*s") != 0 ||
	(creator != "System" &&
	 !::find_object(ACCESSD)->access(oname, path, WRITE_ACCESS))) {
	error("Access denied");
    }

    fcreator = driver->creator(path);
    rsrcd = ::find_object(RSRCD);
    rsrc = rsrcd->rsrc_get(fcreator, "filequota");
    if (creator != "System" && rsrc[RSRC_USAGE] >= rsrc[RSRC_MAX] &&
	rsrc[RSRC_MAX] >= 0) {
	error("File quota exceeded");
    }

    size = driver->file_size(path);
    catch {
	rlimits (-1; -1) {
	    result = ::write_file(path, str, offset);
	    if (result != 0 && (size=driver->file_size(path) - size) != 0) {
		rsrcd->rsrc_incr(fcreator, "filequota", nil, size);
	    }
	}
    } : error(TLSVAR2);

    return result;
}

/*
 * NAME:	remove_file()
 * DESCRIPTION:	remove a file
 */
static int remove_file(string path)
{
    string oname;
    object driver;
    int size, result;

    CHECKARG(path, 1, "remove_file");
    if (!this_object()) {
	error("Permission denied");
    }

    oname = object_name(this_object());
    driver = ::find_object(DRIVER);
    path = driver->normalize_path(path, oname + "/..", creator);
    if (sscanf(path, "/kernel/%*s") != 0 ||
	sscanf(path, "/include/kernel/%*s") != 0 ||
	(creator != "System" &&
	 !::find_object(ACCESSD)->access(oname, path, WRITE_ACCESS))) {
	error("Access denied");
    }

    size = driver->file_size(path);
    catch {
	rlimits (-1; -1) {
	    result = ::remove_file(path);
	    if (result != 0 && size != 0) {
		::find_object(RSRCD)->rsrc_incr(driver->creator(path),
						"filequota", nil, -size);
	    }
	}
    } : error(TLSVAR2);
    return result;
}

/*
 * NAME:	rename_file()
 * DESCRIPTION:	rename a file
 */
static int rename_file(string from, string to)
{
    string oname, fcreator, tcreator;
    object driver, accessd, rsrcd;
    int size, result, *rsrc;

    CHECKARG(from, 1, "rename_file");
    CHECKARG(to, 2, "rename_file");
    if (!this_object()) {
	error("Permission denied");
    }

    oname = object_name(this_object());
    driver = ::find_object(DRIVER);
    from = driver->normalize_path(from, oname + "/..", creator);
    to = driver->normalize_path(to, oname + "/..", creator);
    accessd = ::find_object(ACCESSD);
    if (sscanf(from + "/", "/kernel/%*s") != 0 ||
	sscanf(to, "/kernel/%*s") != 0 ||
	sscanf(from + "/", "/include/kernel/%*s") != 0 || from == "/include" ||
	sscanf(to, "/include/kernel/%*s") != 0 ||
	(creator != "System" &&
	 (!accessd->access(oname, from, WRITE_ACCESS) ||
	  !accessd->access(oname, to, WRITE_ACCESS)))) {
	error("Access denied");
    }

    fcreator = driver->creator(from);
    tcreator = driver->creator(to);
    size = driver->file_size(from, TRUE);
    rsrcd = ::find_object(RSRCD);
    rsrc = rsrcd->rsrc_get(tcreator, "filequota");
    if (size != 0 && fcreator != tcreator && creator != "System" &&
	rsrc[RSRC_USAGE] >= rsrc[RSRC_MAX] && rsrc[RSRC_MAX] >= 0) {
	error("File quota exceeded");
    }

    catch {
	rlimits (-1; -1) {
	    result = ::rename_file(from, to);
	    if (result != 0 && fcreator != tcreator) {
		rsrcd->rsrc_incr(tcreator, "filequota", nil, size);
		rsrcd->rsrc_incr(fcreator, "filequota", nil, -size);
	    }
	}
    } : error(TLSVAR2);
    return result;
}

/*
 * NAME:	get_dir()
 * DESCRIPTION:	get a directory listing
 */
static mixed **get_dir(string path)
{
    string oname, *names, dir;
    mixed **list, *olist;
    int i, sz;

    CHECKARG(path, 1, "get_dir");
    if (!this_object()) {
	error("Permission denied");
    }

    oname = object_name(this_object());
    path = ::find_object(DRIVER)->normalize_path(path, oname + "/..", creator);
    if (creator != "System" &&
	!::find_object(ACCESSD)->access(oname, path, READ_ACCESS)) {
	error("Access denied");
    }

    list = ::get_dir(path);
    names = explode(path, "/");
    dir = implode(names[.. sizeof(names) - 2], "/");
    names = list[0];
    olist = allocate(sz = sizeof(names));
    if (sscanf(path, "%*s" + INHERITABLE_SUBDIR) != 0 ||
	sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s#") == 1 ||
	sscanf(path, "%*s" + LIGHTWEIGHT_SUBDIR) != 0) {
	/* class objects */
	for (i = sz; --i >= 0; ) {
	    path = dir + "/" + names[i];
	    if ((sz=strlen(path)) >= 2 && path[sz - 2 ..] == ".c" &&
		::find_object(path[.. sz - 3])) {
		olist[i] = TRUE;
	    }
	}
    } else {
	/* ordinary objects */
	for (i = sz; --i >= 0; ) {
	    object obj;

	    path = dir + "/" + names[i];
	    if ((sz=strlen(path)) >= 2 && path[sz - 2 ..] == ".c" &&
		(obj=::find_object(path[.. sz - 3]))) {
		olist[i] = obj;
	    }
	}
    }
    return list + ({ olist });
}

/*
 * NAME:	file_info()
 * DESCRIPTION:	get info for a single file
 */
static mixed *file_info(string path)
{
    string name, *files;
    mixed *info;
    int i, sz;
    object obj;

    CHECKARG(path, 1, "file_info");
    if (!this_object()) {
	error("Permission denied");
    }

    name = object_name(this_object());
    path = ::find_object(DRIVER)->normalize_path(path, name + "/..", creator);
    if (creator != "System" &&
	!::find_object(ACCESSD)->access(name, path, READ_ACCESS)) {
	error("Access denied");
    }

    info = ::get_dir(path);
    if (path == "/") {
	name = ".";
    } else {
	files = explode(path, "/");
	name = files[sizeof(files) - 1];
    }
    files = info[0];
    sz = sizeof(files);
    if (sz <= 1) {
	if (sz == 0 || files[0] != name) {
	    return nil;	/* file does not exist */
	}
    } else {
	/* name is a pattern: find in file list */
	for (i = 0; name != files[i]; ) {
	    if (++i == sz) {
		return nil;	/* file does not exist */
	    }
	}
    }
    info = ({ info[1][i], info[2][i], nil });
    if ((sz=strlen(path)) >= 2 && path[sz - 2 ..] == ".c" &&
	(obj=::find_object(path[.. sz - 3]))) {
	info[2] = (sscanf(path, "%*s" + INHERITABLE_SUBDIR) != 0 ||
		   sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s#") == 1 ||
		   sscanf(path, "%*s" + LIGHTWEIGHT_SUBDIR) != 0) ? TRUE : obj;
    }
    return info;
}

/*
 * NAME:	make_dir()
 * DESCRIPTION:	create a directory
 */
static int make_dir(string path)
{
    string oname, fcreator;
    object driver, rsrcd;
    int result, *rsrc;

    CHECKARG(path, 1, "make_dir");
    if (!this_object()) {
	error("Permission denied");
    }

    oname = object_name(this_object());
    driver = ::find_object(DRIVER);
    path = driver->normalize_path(path, oname + "/..", creator);
    if (sscanf(path, "/kernel/%*s") != 0 ||
	sscanf(path, "/include/kernel/%*s") != 0 ||
	(creator != "System" &&
	 !::find_object(ACCESSD)->access(oname, path, WRITE_ACCESS))) {
	error("Access denied");
    }

    fcreator = driver->creator(path + "/");
    rsrcd = ::find_object(RSRCD);
    rsrc = rsrcd->rsrc_get(fcreator, "filequota");
    if (creator != "System" && rsrc[RSRC_USAGE] >= rsrc[RSRC_MAX] &&
	rsrc[RSRC_MAX] >= 0) {
	error("File quota exceeded");
    }

    catch {
	rlimits (-1; -1) {
	    result = ::make_dir(path);
	    if (result != 0) {
		rsrcd->rsrc_incr(fcreator, "filequota", nil, 1);
	    }
	}
    } : error(TLSVAR2);
    return result;
}

/*
 * NAME:	remove_dir()
 * DESCRIPTION:	remove a directory
 */
static int remove_dir(string path)
{
    string oname;
    object driver;
    int result;

    CHECKARG(path, 1, "remove_dir");
    if (!this_object()) {
	error("Permission denied");
    }

    oname = object_name(this_object());
    driver = ::find_object(DRIVER);
    path = driver->normalize_path(path, oname + "/..", creator);
    if (sscanf(path, "/kernel/%*s") != 0 ||
	sscanf(path, "/include/kernel/%*s") != 0 ||
	(creator != "System" &&
	 !::find_object(ACCESSD)->access(oname, path, WRITE_ACCESS))) {
	error("Access denied");
    }

    catch {
	rlimits (-1; -1) {
	    result = ::remove_dir(path);
	    if (result != 0) {
		::find_object(RSRCD)->rsrc_incr(driver->creator(path + "/"),
						"filequota", nil, -1);
	    }
	}
    } : error(TLSVAR2);
    return result;
}

/*
 * NAME:	restore_object()
 * DESCRIPTION:	restore the state of an object
 */
static int restore_object(string path)
{
    string oname;

    CHECKARG(path, 1, "restore_object");
    if (!this_object()) {
	error("Permission denied");
    }

    oname = object_name(this_object());
    path = ::find_object(DRIVER)->normalize_path(path, oname + "/..", creator);
    if (creator != "System" &&
	!::find_object(ACCESSD)->access(oname, path, READ_ACCESS)) {
	error("Access denied");
    }

    return ::restore_object(path);
}

/*
 * NAME:	save_object()
 * DESCRIPTION:	save the state of an object
 */
static void save_object(string path)
{
    string oname, fcreator;
    object driver, rsrcd;
    int size, *rsrc;

    CHECKARG(path, 1, "save_object");
    if (!this_object()) {
	error("Permission denied");
    }

    oname = object_name(this_object());
    driver = ::find_object(DRIVER);
    path = driver->normalize_path(path, oname + "/..", creator);
    if ((sscanf(path, "/kernel/%*s") != 0 &&
	 sscanf(oname, "/kernel/%*s") == 0) ||
	sscanf(path, "/include/kernel/%*s") != 0 ||
	(creator != "System" &&
	 !::find_object(ACCESSD)->access(oname, path, WRITE_ACCESS))) {
	error("Access denied");
    }

    fcreator = driver->creator(path);
    rsrcd = ::find_object(RSRCD);
    rsrc = rsrcd->rsrc_get(fcreator, "filequota");
    if (creator != "System" && rsrc[RSRC_USAGE] >= rsrc[RSRC_MAX] &&
	rsrc[RSRC_MAX] >= 0) {
	error("File quota exceeded");
    }

    size = driver->file_size(path);
    catch {
	rlimits (-1; -1) {
	    ::save_object(path);
	    if ((size=driver->file_size(path) - size) != 0) {
		rsrcd->rsrc_incr(fcreator, "filequota", nil, size);
	    }
	}
    } : error(TLSVAR2);
}

/*
 * NAME:	editor()
 * DESCRIPTION:	pass a command to the editor
 */
static string editor(varargs string cmd)
{
    object rsrcd, driver;
    string result;
    mixed *info;

    if (creator != "System" || !this_object() ||
	sscanf(object_name(this_object()), "%*s#-1") != 0) {
	error("Permission denied");
    }

    catch {
	rlimits (-1; -1) {
	    rsrcd = ::find_object(RSRCD);
	    if (!query_editor(this_object())) {
		::find_object(USERD)->add_editor(this_object());
	    }
	    driver = ::find_object(DRIVER);

	    TLSVAR2 = nil;
	    result = (cmd) ? ::editor(cmd) : ::editor();
	    info = TLSVAR2;

	    if (!query_editor(this_object())) {
		::find_object(USERD)->remove_editor(this_object());
	    }
	    if (info) {
		rsrcd->rsrc_incr(driver->creator(info[0]), "filequota", nil,
				 driver->file_size(info[0]) - info[1]);
	    }
	}
    } : error(TLSVAR2);
    return result;
}
