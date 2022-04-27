# include <kernel/kernel.h>
# include <trace.h>
# include <type.h>
# include "tls.h"

# define SYSTEMAUTO	"/usr/System/lib/auto"

inherit auto AUTO;
inherit SYSTEMAUTO;


object driver;

/*
 * NAME:	create()
 * DESCRIPTION:	initialize this object
 */
static void create()
{
    driver = find_object(DRIVER);
    driver->set_error_manager(this_object());
}

/*
 * NAME:	spaces()
 * DESCRIPTION:	returns the number of spaces requested
 */
private string spaces(int width)
{
    string str;

    /* 63 spaces */
    str = "                                                               ";
    while (strlen(str) < width) {
        str += str;
    }
    return str[.. width - 1];
}

/*
 * NAME:	pad_left()
 * DESCRIPTION:	pad with spaces to the left
 */
private string pad_left(mixed str, int width)
{
    str = (string) str;
    if (strlen(str) >= width) {
	return str;
    }
    str = spaces(width) + str;
    return str[strlen(str) - width ..];
}

/*
 * NAME:	pad_right()
 * DESCRIPTION:	pad with spaces to the right
 */
private string pad_right(mixed str, int width)
{
    str = (string) str;
    if (strlen(str) >= width) {
	return str;
    }
    return (str + spaces(width))[.. width - 1];
}

/*
 * NAME:	runtime_error()
 * DESCRIPTION:	report a runtime error
 */
void runtime_error(string error, int caught, mixed **trace)
{
    if (previous_object() == driver) {
	int i, j, sz, maxlen, line;
	string progname, objname, last_obj, str;
	mixed *ftrace, *lines;
	object user;

	auto::tls_set(TLS_ARGUMENTS, nil);

	if (caught != 0) {
	    error += " [caught]";
	}
	sz = sizeof(trace) - 1;
	lines = allocate(sz * 2);
	j = 0;
	maxlen = 0;

	for (i = 0; i < sz; i++) {
	    ftrace = trace[i];
	    objname = ftrace[TRACE_OBJNAME];
	    progname = ftrace[TRACE_PROGNAME];
	    str = ftrace[TRACE_FUNCTION];
	    if (progname != AUTO &&
		(progname != SYSTEMAUTO || str[.. 2] != "_F_")) {
		if (objname != last_obj) {
		    lines[j++] = objname;
		    last_obj = objname;
		}

		line = ftrace[TRACE_LINE];
		str = ((line != 0) ? pad_left(line, 5) : "     ") +
		      ((i + 1 == caught) ? " * " : "   ") + str;
		if (strlen(str) > maxlen) {
		    maxlen = strlen(str);
		}
		lines[j++] = ({ str, progname });
	    }
	}

	str = error + "\n";
	for (i = 0; i < j; i++) {
	    if (typeof(lines[i]) == T_STRING) {
		str += spaces(maxlen + 1) + lines[i];
	    } else {
		str += pad_right(lines[i][0], maxlen);
		if (lines[i][1]) {
		    str += "    " + lines[i][1];
		}
	    }
	    str += "\n";
	}

	driver->message(str);
	if (caught == 0 && (user=this_user())) {
	    user->message(str);
	}
    }
}

/*
 * NAME:	atomic_error()
 * DESCRIPTION:	report a runtime error in atomic code
 */
void atomic_error(string error, int atom, mixed **trace)
{
    if (previous_object() == driver) {
	int i, j, sz, maxlen, line;
	string progname, objname, last_obj, str;
	mixed *ftrace, *lines;

	sz = sizeof(trace) - 1;
	lines = allocate(sz * 2);
	j = 0;
	maxlen = 0;

	for (i = atom; i < sz; i++) {
	    ftrace = trace[i];
	    objname = ftrace[TRACE_OBJNAME];
	    progname = ftrace[TRACE_PROGNAME];
	    str = ftrace[TRACE_FUNCTION];
	    if (progname != AUTO &&
		(progname != SYSTEMAUTO || str[.. 2] != "_F_")) {
		if (objname != last_obj) {
		    lines[j++] = objname;
		    last_obj = objname;
		}

		line = ftrace[TRACE_LINE];
		str = ((line != 0) ? pad_left(line, 5) : "     ") + "   " +
		      ftrace[TRACE_FUNCTION];
		if (strlen(str) > maxlen) {
		    maxlen = strlen(str);
		}
		lines[j++] = ({ str, progname });
	    }
	}

	str = error + " [atomic]\n";
	for (i = 0; i < j; i++) {
	    if (typeof(lines[i]) == T_STRING) {
		str += spaces(maxlen + 1) + lines[i];
	    } else {
		str += pad_right(lines[i][0], maxlen);
		if (lines[i][1]) {
		    str += "    " + lines[i][1];
		}
	    }
	    str += "\n";
	}

	driver->message(str);
    }
}

/*
 * NAME:	compile_error()
 * DESCRIPTION:	report a compile-time error
 */
void compile_error(string file, int line, string error)
{
    if (previous_object() == driver) {
	string str;
	object user;

	str = file + ", " + line + ": " + error;
	if (auto::tls_get(TLS_COMPILE_ERRORS)) {
	    send_atomic_message(str);
	}
	str += "\n";
	driver->message(str);
	user = this_user();
	if (user) {
	    user->message(str);
	}
    }
}
