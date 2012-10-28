# include <kernel/kernel.h>
# include <messages.h>
# include <trace.h>
# include <type.h>

inherit "~/lib/auto";

private inherit "/lib/util/string";


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
 * NAME:	runtime_error()
 * DESCRIPTION:	report a runtime error
 */
void runtime_error(string error, int caught, mixed **trace)
{
    if (previous_object() == driver) {
	int i, j, sz, maxlen, line;
	string progname, objname, last_obj, str;
	mixed *ftrace, *lines;

	if (caught != 0) {
	    error += " [caught]";
	}
	sz = sizeof(trace) - 1;
	lines = allocate(sz * 2);
	j = 0;
	maxlen = 0;

	for (i = 0; i < sz; i++) {
	    ftrace = trace[i];
	    progname = ftrace[TRACE_PROGNAME];
	    objname = ftrace[TRACE_OBJNAME];
	    if (sscanf(progname, "/kernel/%*s") == 0) {
		if (objname != last_obj) {
		    lines[j++] = objname;
		    last_obj = objname;
		}

		line = ftrace[TRACE_LINE];
		str = ((line != 0) ? pad_left(line, 5) : "     ") +
		      ((i + 1 == caught) ? " * " : "   ") +
		      ftrace[TRACE_FUNCTION];
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
	if (caught == 0 && this_user()) {
	    this_user()->message(MSG_FORMATTED, str);
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
	    progname = ftrace[TRACE_PROGNAME];
	    objname = ftrace[TRACE_OBJNAME];
	    if (sscanf(progname, "/kernel/%*s") == 0) {
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

	str = file + ", " + line + ": " + error + "\n";
	driver->message(str);
	if (this_user()) {
	    this_user()->message(MSG_FORMATTED, str);
	}
    }
}
