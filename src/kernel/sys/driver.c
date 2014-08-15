# include <kernel/kernel.h>
# include <kernel/rsrc.h>
# include <kernel/access.h>
# include <kernel/user.h>
# include <status.h>
# include <trace.h>
# include <type.h>

# define TLS(tls, n)	tls[-1 - n]
# define TLSVAR(n)	call_trace()[1][TRACE_FIRSTARG][-1 - n]


object rsrcd;		/* resource manager object */
object accessd;		/* access manager object */
object userd;		/* user manager object */
object initd;		/* init manager object */
object objectd;		/* object manager object */
object errord;		/* error manager object */

/*
 * NAME:	creator()
 * DESCRIPTION:	get creator of file
 */
string creator(string file)
{
    return (sscanf(file, "/kernel/%*s") != 0) ? "System" :
	    (sscanf(file, USR_DIR + "/%s/", file) != 0) ? file : nil;
}

/*
 * NAME:	normalize_path()
 * DESCRIPTION:	reduce a path to its minimal absolute form
 */
string normalize_path(string file, varargs string dir, string creator)
{
    string *path;
    int i, j, sz;

    if (strlen(file) == 0) {
	if (!dir) {
	    dir = object_name(previous_object()) + "/..";
	}
	file = dir;
    }
    switch (file[0]) {
    case '~':
	/* ~path */
	if (!creator) {
	    creator = creator(object_name(previous_object()));
	}
	if (creator && (strlen(file) == 1 || file[1] == '/')) {
	    file = USR_DIR + "/" + creator + file[1 ..];
	} else {
	    file = USR_DIR + "/" + file[1 ..];
	}
	/* fall through */
    case '/':
	/* absolute path */
	if (sscanf(file, "%*s//") == 0 && sscanf(file, "%*s/.") == 0) {
	    return file;	/* no changes */
	}
	path = explode(file, "/");
	break;

    default:
	/* relative path */
	if (sscanf(file, "%*s//") == 0 && sscanf(file, "%*s/.") == 0 && dir &&
	    sscanf(dir, "%*s/..") == 0) {
	    /*
	     * simple relative path
	     */
	    return dir + "/" + file;
	}
	/* fall through */
    case '.':
	/*
	 * complex relative path
	 */
	if (!dir) {
	    dir = object_name(previous_object()) + "/..";
	}
	path = explode(dir + "/" + file, "/");
	break;
    }

    for (i = 0, j = -1, sz = sizeof(path); i < sz; i++) {
	switch (path[i]) {
	case "..":
	    if (j >= 0) {
		--j;
	    }
	    /* fall through */
	case "":
	case ".":
	    continue;
	}
	path[++j] = path[i];
    }

    return "/" + implode(path[.. j], "/");
}

/*
 * NAME:	dir_size()
 * DESCRIPTION:	get the size of all files in a directory
 */
private int dir_size(string file)
{
    mixed **info;
    int *sizes, size, i, sz;

    info = get_dir(file + "/*");
    sizes = info[1];
    size = 1;		/* 1K for directory itself */
    i = sizeof(sizes);
    while (--i >= 0) {
	sz = sizes[i];
	size += (sz > 0) ?
		 (sz + 1023) >> 10 :
		 (sz == 0) ? 1 : dir_size(file + "/" + info[0][i]);
    }

    return size;
}

/*
 * NAME:	file_size()
 * DESCRIPTION:	get the size of a file in K, or 0 if the file doesn't exist
 */
int file_size(string file, varargs int dir)
{
    if (KERNEL() || SYSTEM()) {
	mixed **info;
	string *files, name;
	int i, sz;

	if (file == "/") {
	    file = ".";
	}
	info = get_dir(file);
	files = explode(file, "/");
	name = files[sizeof(files) - 1];
	files = info[0];
	i = 0;
	sz = sizeof(files);

	if (sz <= 1) {
	    if (sz == 0 || files[0] != name) {
		return 0;	/* file does not exist */
	    }
	} else {
	    /* name is a pattern: find in file list */
	    while (name != files[i]) {
		if (++i == sz) {
		    return 0;	/* file does not exist */
		}
	    }
	}

	i = info[1][i];
	return (i > 0) ?
		(i + 1023) >> 10 :
		(i == 0) ? 1 : (i == -2 && dir) ? dir_size(file) : 0;
    }
}

/*
 * NAME:	set_object_manager()
 * DESCRIPTION:	set the object manager
 */
void set_object_manager(object obj)
{
    if (SYSTEM()) {
	objectd = obj;
    }
}

/*
 * NAME:	set_error_manager()
 * DESCRIPTION:	set the error manager
 */
void set_error_manager(object obj)
{
    if (SYSTEM()) {
	errord = obj;
    }
}

/*
 * NAME:	_compile()
 * DESCRIPTION:	low-level compilation function
 */
private atomic object _compile(string path, string creator,
			       varargs string *source)
{
    object obj;

    TLSVAR(2) = ({ path });
    obj = (source) ? compile_object(path, source...) : compile_object(path);
    rsrcd->rsrc_incr(creator, "objects", nil, 1);
    if (objectd) {
	objectd->compile(creator, path, ({ }), TLSVAR(2)[1 ..]...);
    }
    return obj;
}

/*
 * NAME:	compiling()
 * DESCRIPTION:	object being compiled
 */
void compiling(string path)
{
    if (previous_program() == AUTO) {
	if (path != AUTO && path != DRIVER && !find_object(AUTO)) {
	    string err;

	    if (objectd) {
		objectd->compiling(AUTO);
	    }
	    err = catch(_compile(AUTO, "System"));
	    if (err) {
		if (objectd) {
		    objectd->compile_failed("System", AUTO);
		}
		error(err);
	    }
	}
	if (objectd) {
	    objectd->compiling(path);
	}
	TLSVAR(2) = ({ path });
    }
}

/*
 * NAME:	compile()
 * DESCRIPTION:	object compiled
 */
void compile(string path, string owner, string source...)
{
    if (previous_program() == AUTO) {
	if (objectd) {
	    objectd->compile(owner, path, source, TLSVAR(2)[1 ..]...);
	}
    }
}

/*
 * NAME:	compile_failed()
 * DESCRIPTION:	object couldn't be compiled
 */
void compile_failed(string path, string owner)
{
    if (previous_program() == AUTO) {
	if (objectd) {
	    objectd->compile_failed(owner, path);
	}
    }
}

/*
 * NAME:	clone()
 * DESCRIPTION:	object cloned
 */
void clone(object obj, string owner)
{
    if (objectd && previous_program() == AUTO) {
	objectd->clone(owner, obj);
    }
}

/*
 * NAME:	destruct()
 * DESCRIPTION:	object about to be destructed
 */
void destruct(string path, string owner)
{
    if (objectd && previous_program() == AUTO) {
	objectd->destruct(owner, path);
    }
}

/*
 * NAME:	query_owner()
 * DESCRIPTION:	return owner of driver object
 */
string query_owner()
{
    return "System";
}


/*
 * NAME:	message()
 * DESCRIPTION:	show message
 */
void message(string str)
{
    if (KERNEL() || SYSTEM()) {
	send_message(ctime(time())[4 .. 18] + " ** " + str);
    }
}

/*
 * NAME:	load()
 * DESCRIPTION:	find or compile object
 */
private object load(string path)
{
    object obj;

    obj = find_object(path);
    if (obj) {
	return obj;
    }
    TLSVAR(2) = ({ path });
    return compile_object(path);
}

/*
 * NAME:	_initialize()
 * DESCRIPTION:	initialize system, with proper TLS on the stack
 */
private void _initialize(mapping tls)
{
    string *users;
    int i;

    message(status()[ST_VERSION] + "\n");
    message("Initializing...\n");

    /* load initial objects */
    load(AUTO);
    call_other(rsrcd = load(RSRCD), "???");
    load(RSRCOBJ);

    /* initialize some resources */
    rsrcd->set_rsrc("stack",	        50, 0, 0);
    rsrcd->set_rsrc("ticks",	    250000, 0, 0);

    /* create initial resource owners */
    rsrcd->add_owner("System");
    rsrcd->rsrc_incr("System", "filequota", nil,
		     dir_size("/kernel") +
		     file_size(USR_DIR + "/System", TRUE));
    rsrcd->add_owner(nil);	/* Ecru */
    rsrcd->rsrc_incr(nil, "filequota", nil,
		     file_size("/doc", TRUE) + file_size("/include", TRUE));

    /* load remainder of manager objects */
    call_other(accessd = load(ACCESSD), "???");
    call_other(userd = load(USERD), "???");
    call_other(load(DEFAULT_WIZTOOL), "???");

    /* correct object count */
    rsrcd->rsrc_incr("System", "objects", nil, 7);

    /* initialize other users as resource owners */
    users = (accessd->query_users() - ({ "System" })) | ({ "admin" });
    for (i = sizeof(users); --i >= 0; ) {
	rsrcd->add_owner(users[i]);
	rsrcd->rsrc_incr(users[i], "filequota", nil,
			 file_size(USR_DIR + "/" + users[i], TRUE));
    }

    /* system-specific initialization */
    if (file_size(USR_DIR + "/System/initd.c") != 0) {
	initd = rsrcd->initd();
	call_other(initd, "???");
    }
}

/*
 * NAME:	initialize()
 * DESCRIPTION:	first function called at system startup
 */
static void initialize()
{
    catch {
	_initialize(([ ]));
	message("Initialization complete.\n\n");
    } : {
	message("Initialization failed.\n");
	shutdown();
    }
}

/*
 * NAME:	prepare_reboot()
 * DESCRIPTION:	prepare for a snapshot
 */
void prepare_reboot()
{
    if (KERNEL()) {
	if (initd) {
	    initd->prepare_reboot();
	}
	rsrcd->prepare_reboot();
	userd->prepare_reboot();
    }
}

/*
 * NAME:	_restored()
 * DESCRIPTION:	re-initialize the system, with proper TLS on the stack
 */
private void _restored(mapping tls, int hotboot)
{
    if (hotboot) {
	if (initd) {
	    initd->hotboot();
	}
    } else {
	rsrcd->reboot();
	userd->reboot();
	if (initd) {
	    initd->reboot();
	}
    }
}

/*
 * NAME:	restored()
 * DESCRIPTION:	re-initialize system after a restore
 */
static void restored(varargs int hotboot)
{
    message(status()[ST_VERSION] + "\n");

    catch {
	_restored(([ ]), hotboot);
    }

    message("State restored.\n\n");
}

/*
 * NAME:	path_read()
 * DESCRIPTION:	handle an editor read path
 */
static string path_read(string path)
{
    string oname, creator;

    catch {
	path = previous_object()->path_read(path);
    }
    if (path) {
	creator = creator(oname = object_name(previous_object()));
	path = normalize_path(path, oname + "/..", creator);
	return ((creator == "System" ||
		 accessd->access(oname, path, READ_ACCESS)) ? path : nil);
    }
    return nil;
}

/*
 * NAME:	path_write()
 * DESCRIPTION:	handle an editor write path
 */
static string path_write(string path)
{
    string oname, creator;
    int *rsrc, size;

    catch {
	path = previous_object()->path_write(path);
    }
    if (path) {
	creator = creator(oname = object_name(previous_object()));
	path = normalize_path(path, oname + "/..", creator);
	rsrc = rsrcd->rsrc_get(creator, "filequota");
	if (sscanf(path, "/kernel/%*s") == 0 &&
	    sscanf(path, "/include/kernel/%*s") == 0 &&
	    (creator == "System" ||
	     (accessd->access(oname, path, WRITE_ACCESS) &&
	      (rsrc[RSRC_USAGE] < rsrc[RSRC_MAX] || rsrc[RSRC_MAX] < 0)))) {
	    TLSVAR(1) = ({ path, file_size(path) });
	    return path;
	}
    }
    return nil;
}

/*
 * NAME:	call_object()
 * DESCRIPTION:	get an object for call_other's first (string) argument
 */
static object call_object(string path)
{
    if (path[0] != '/') {
	path = normalize_path(path);
    }
    if (sscanf(path, "%*s/lib/") != 0 ||
	(objectd && objectd->forbid_call(path))) {
	error("Illegal use of call_other");
    }
    return find_object(path);
}

/*
 * NAME:	object_type()
 * DESCRIPTION:	return normalized object type
 */
static string object_type(string file, string type)
{
    return normalize_path(type, file + "/..", creator(file));
}

/*
 * NAME:	_touch()
 * DESCRIPTION:	touch an object that has been flagged with call_touch()
 */
private int _touch(mapping tls, object obj, string func)
{
    return objectd->touch(obj, func);
}

/*
 * NAME:	touch()
 * DESCRIPTION:	wrapper for _touch()
 */
static int touch(object obj, string func)
{
    mapping tls;
    string prog;

    if (objectd) {
	if (!previous_object()) {
	    tls = ([ ]);
	} else if (KERNEL()) {
	    prog = function_object(func, obj);
	    if (prog && sscanf(prog, "/kernel/%*s") != 0 &&
		status()[ST_STACKDEPTH] < 0) {
		/*
		 * protected kernel-to-kernel calls leave the object
		 * "untouched"
		 */
		return TRUE;
	    }
	}

	return _touch(tls, obj, func);
    }
    return FALSE;
}

/*
 * NAME:	inherit_program()
 * DESCRIPTION:	inherit a program, compiling it if needed
 */
static object inherit_program(string from, string path, int priv)
{
    string creator;
    object obj;
    mixed str;

    path = normalize_path(path, from + "/..", creator = creator(from));
    if (sscanf(path, "%*s/lib/") == 0 ||
	(sscanf(path, "/kernel/%*s") != 0 && creator != "System") ||
	!accessd->access(from, path, READ_ACCESS)) {
	return nil;
    }

    if (objectd) {
	str = objectd->inherit_program(from, path, priv);
	if (sscanf(from, "/kernel/%*s") != 0) {
	    str = nil;
	} else {
	    switch (typeof(str)) {
	    case T_STRING:
		path = str;
		str = nil;
		break;

	    case T_ARRAY:
		break;

	    default:
		return nil;
	    }
	}
    }

    obj = find_object(path);
    if (!obj) {
	int *rsrc;

	creator = creator(path);
	rsrc = rsrcd->rsrc_get(creator, "objects");
	if (rsrc[RSRC_USAGE] >= rsrc[RSRC_MAX] && rsrc[RSRC_MAX] >= 0) {
	    error("Too many objects");
	}

	if (objectd) {
	    objectd->compiling(path);
	}
	TLSVAR(2) = ({ path });
	obj = (str) ? compile_object(path, str...) : compile_object(path);
	rsrcd->rsrc_incr(creator, "objects", nil, 1);
	if (objectd) {
	    objectd->compile(creator, path, ({ }), TLSVAR(2)[1 ..]...);

	    objectd->compiling(from);
	}
	TLSVAR(2) = ({ from });
    } else {
	TLSVAR(2) += ({ path });
    }
    return obj;
}

/*
 * NAME:	include_file()
 * DESCRIPTION:	translate and return an include path, or the contents of the
 *		file as an array of strings
 */
static mixed include_file(string from, string path)
{
    if (strlen(path) != 0 && path[0] != '~' && sscanf(path, "%*s/../") == 0 &&
	(sscanf(path, "/include/%*s") != 0 || sscanf(path, "%*s/") == 0)) {
	/*
	 * safe include: skip access check
	 */
	if (path[0] != '/') {
	    path = normalize_path(path, from + "/..", creator(from));
	}
    } else {
	path = normalize_path(path, from + "/..", creator(from));
	if (!accessd->access(from, path, READ_ACCESS)) {
	    return nil;
	}
    }
    if (objectd) {
	mixed result;

	result = objectd->include_file(TLSVAR(2)[0], from, path);
	if (sscanf(from, "/kernel/%*s") == 0) {
	    return result;
	}
    }
    return path;
}

/*
 * NAME:	remove_program()
 * DESCRIPTION:	the last reference to a program is removed
 */
static void remove_program(string path, int timestamp, int index)
{
    string creator;

    creator = creator(path);
    if (path != RSRCOBJ) {
	rsrcd->rsrc_incr(creator, "objects", nil, -1);
    }
    if (objectd) {
	objectd->remove_program(creator, path, timestamp, index);
    }
}

/*
 * NAME:	recompile()
 * DESCRIPTION:	recompile an inherited object
 */
static void recompile(object obj)
{
    if (objectd) {
	string name;

	name = object_name(obj);
	objectd->destruct(creator(name), name);
    }
    destruct_object(obj);
}

/*
 * NAME:	telnet_connect()
 * DESCRIPTION:	return a telnet connection user object
 */
static object telnet_connect(int port)
{
    return userd->telnet_connection(([ ]), port);
}

/*
 * NAME:	binary_connect()
 * DESCRIPTION:	return a binary connection user object
 */
static object binary_connect(int port)
{
    return userd->binary_connection(([ ]), port);
}

/*
 * NAME:	_interrupt()
 * DESCRIPTION:	handle interrupt signal, with proper TLS on the stack
 */
private void _interrupt(mapping tls)
{
    message("Interrupt.\n");

# ifdef SYS_PERSISTENT
    prepare_reboot();
    dump_state(1);
# endif
    shutdown();
}

/*
 * NAME:	interrupt()
 * DESCRIPTION:	called when a kill signal is sent to the server
 */
static void interrupt()
{
    _interrupt(([ ]));
}

/*
 * NAME:	_runtime_error()
 * DESCRIPTION:	handle runtime error, with proper TLS on the stack
 */
private void _runtime_error(mapping tls, string str, int caught, int ticks,
			    mixed **trace, object user)
{
    string line, func, progname, objname;
    int i, sz, len;

    i = sz = sizeof(trace) - 1;

    if (ticks >= 0) {
	mixed *limits;

	limits = TLS(tls, 0);
	while (--i >= caught) {
	    if (trace[i][TRACE_FUNCTION] == "_F_call_limited" &&
		trace[i][TRACE_PROGNAME] == AUTO) {
		ticks = rsrcd->update_ticks(limits, ticks);
		if (ticks < 0) {
		    break;
		}
		limits = limits[LIM_NEXT];
	    }
	}
	TLS(tls, 0) = limits;
    }

    if (errord) {
	errord->runtime_error(str, caught, trace);
    } else {
	if (caught != 0) {
	    str += " [caught]";
	}
	str += "\n";

	for (i = 0; i < sz; i++) {
	    progname = trace[i][TRACE_PROGNAME];
	    len = trace[i][TRACE_LINE];
	    if (len == 0) {
		line = "    ";
	    } else {
		line = "    " + len;
		line = line[strlen(line) - 4 ..];
	    }

	    func = trace[i][TRACE_FUNCTION];
	    len = strlen(func);
	    if (progname == AUTO && i != sz - 1) {
		switch (func[.. 2]) {
		case "bad":
		case "_F_":
		    continue;
		}
	    }
	    if (len < 17) {
		func += "                 "[len ..];
	    }

	    objname = trace[i][TRACE_OBJNAME];
	    if (progname != objname) {
		len = strlen(progname);
		if (len < strlen(objname) && progname == objname[.. len - 1] &&
		    objname[len] == '#') {
		    objname = objname[len ..];
		}
		str += line + " " + func + " " + progname + " (" + objname +
		       ")\n";
	    } else {
		str += line + " " + func + " " + progname + "\n";
	    }
	}

	message(str);
	if (caught == 0 && user) {
	    user->message(str);
	}
    }
}

/*
 * NAME:	runtime_error()
 * DESCRIPTION:	log a runtime error
 */
static string runtime_error(string str, int caught, int ticks)
{
    object user;
    string *messages;
    int i, sz;
    mixed **trace;
    mapping tls;

    user = this_user();
    while (user && function_object("query_user", user) == LIB_CONN) {
	user = user->query_user();
    }
    messages = explode(str, "\n");
    for (i = 0, sz = sizeof(messages) - 1; i < sz; i++) {
	string file;
	int line;

	if (sscanf(messages[i], "\0%s\0%d\0%s", file, line, str) != 0) {
	    if (errord) {
		errord->compile_error(file, line, str);
	    } else {
		send_message(file += ", " + line + ": " + str + "\n");
		if (user) {
		    user->message(file);
		}
	    }
	    messages[i] = nil;
	} else {
	    messages[i] = messages[i][1 ..];
	}
    }
    str = messages[sz];
    messages[i] = nil;
    trace = call_trace();
    TLS(trace[1][TRACE_FIRSTARG], 4) = messages - ({ nil });

    if (sizeof(trace) == 1) {
	/* top-level error */
	tls = ([ ]);
    } else {
	tls = trace[1][TRACE_FIRSTARG];
	trace[1][TRACE_FIRSTARG] = nil;
	if (caught <= 1) {
	    caught = 0;		/* ignore top-level catch */
	} else if (ticks < 0 && sscanf(trace[caught - 1][TRACE_PROGNAME],
				       "/kernel/%*s") != 0) {
	    return TLS(tls, 1) = str;
	}
    }

    _runtime_error(tls, str, caught, ticks, trace, user);

    return str;
}

/*
 * NAME:	atomic_error()
 * DESCRIPTION:	log a runtime error in atomic code
 */
static string atomic_error(string str, int atom, int ticks)
{
    mixed **trace;
    string *messages, mesg, line, func, progname, objname;
    int i, sz, len;
    object obj;

    trace = call_trace();
    messages = TLS(trace[1][TRACE_FIRSTARG], 3);
    if (messages) {
	mesg = implode(messages, "\n") + "\n" + str;
    } else {
	mesg = str;
    }

    if (sscanf(trace[atom][TRACE_PROGNAME], "/kernel/%*s") == 0) {
	if (errord) {
	    errord->atomic_error(str, atom, trace);
	} else {
	    str += " [atomic]\n";

	    for (i = atom, sz = sizeof(trace) - 1; i < sz; i++) {
		progname = trace[i][TRACE_PROGNAME];
		len = trace[i][TRACE_LINE];
		if (len == 0) {
		    line = "    ";
		} else {
		    line = "    " + len;
		    line = line[strlen(line) - 4 ..];
		}

		func = trace[i][TRACE_FUNCTION];
		len = strlen(func);
		if (progname == AUTO && i != sz - 1) {
		    switch (func[.. 2]) {
		    case "bad":
		    case "_F_":
			continue;
		    }
		}
		if (len < 17) {
		    func += "                 "[len ..];
		}

		objname = trace[i][TRACE_OBJNAME];
		if (progname != objname) {
		    len = strlen(progname);
		    if (len < strlen(objname) &&
			progname == objname[.. len - 1] && objname[len] == '#')
		    {
			objname = objname[len ..];
		    }
		    str += line + " " + func + " " + progname + " (" + objname +
			   ")\n";
		} else {
		    str += line + " " + func + " " + progname + "\n";
		}
	    }

	    message(str);
	}
    }

    return mesg;
}

/*
 * NAME:	compile_error()
 * DESCRIPTION:	deal with a compilation error
 */
static void compile_error(string file, int line, string err)
{
    string *mesg, *messages;
    mapping tls;

    mesg = ({ "\0" + file + "\0" + line + "\0" + err });
    tls = call_trace()[1][TRACE_FIRSTARG];
    messages = TLS(tls, 3);
    if (messages) {
	messages += mesg;
    } else {
	messages = mesg;
    }
    TLS(tls, 3) = messages;
}

/*
 * NAME:	compile_rlimits()
 * DESCRIPTION:	compile-time check on rlimits
 */
static int compile_rlimits(string objname)
{
    /* unlimited resource usage for kernel objects */
    return sscanf(objname, "/kernel/%*s");
}

/*
 * NAME:	runtime_rlimits()
 * DESCRIPTION:	runtime check on rlimits
 */
static int runtime_rlimits(object obj, int maxdepth, int maxticks)
{
    int depth, ticks;

    if (maxdepth != 0) {
	if (maxdepth < 0) {
	    return FALSE;
	}
	depth = status()[ST_STACKDEPTH];
	if (depth >= 0 && maxdepth > depth + 1) {
	    return SYSTEM();
	}
    }
    if (maxticks != 0) {
	if (maxticks < 0) {
	    return SYSTEM();
	}
	ticks = status()[ST_TICKS];
	if (ticks >= 0 && maxticks > ticks) {
	    return SYSTEM();
	}
    }

    return TRUE;
}
