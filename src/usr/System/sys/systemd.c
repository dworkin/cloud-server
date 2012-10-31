# include <kernel/kernel.h>
# include <kernel/rsrc.h>
# include <kernel/user.h>
# include <kernel/access.h>

inherit		"~/lib/auto";
inherit user	API_USER;
inherit access	API_ACCESS;

# define CO_OBJ		0	/* callout object */
# define CO_HANDLE	1	/* handle in object */
# define CO_RELHANDLE	2	/* release handle */
# define CO_PREV	3	/* previous callout */
# define CO_NEXT	4	/* next callout */


object rsrcd;		/* resource daemon */
object suspender;	/* object starting the suspension */
int suspend;		/* -1: suspended  0: not suspended  1: releasing */
mapping suspended;	/* suspended callouts */
mixed *first_suspended;	/* first suspended callout */
mixed *last_suspended;	/* last suspended callout */
object *connections;	/* active connections during suspension */
object user;		/* this_user() at start of suspension */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize System controller
 */
static void create()
{
    user::create();
    access::create();
    rsrcd = find_object(RSRCD);
    rsrcd->set_suspension_manager(this_object());
}

/*
 * NAME:	suspend()
 * DESCRIPTION:	suspend the system
 */
void suspend()
{
    if (SYSTEM()) {
	rlimits (0; -1) {
	    mixed *callout;
	    int i;

	    rsrcd->suspend_callouts();

	    if (suspend > 0) {
		/* stop releasing callouts */
		callout = first_suspended;
		do {
		    if (callout[CO_RELHANDLE] != 0) {
			remove_call_out(callout[CO_RELHANDLE]);
			callout[CO_RELHANDLE] = 0;
		    }
		    callout = callout[CO_NEXT];
		} while (callout);
	    } else {
		suspended = ([ ]);
	    }
	    suspender = previous_object();
	    suspend = -1;

	    /* block input on current connections */
	    connections = query_connections();
	    for (i = sizeof(connections); --i >= 0; ) {
		if (connections[i]->query_mode() == MODE_BLOCK) {
		    connections[i] = nil;
		} else {
		    connections[i]->set_mode(MODE_BLOCK);
		}
	    }

	    /* register current user, and suspend if existing */
	    user = this_user();
	    if (user) {
		user->suspend();
	    }
	}
    }
}

/*
 * NAME:	release()
 * DESCRIPTION:	release system suspension
 */
void release()
{
    if (previous_object() == suspender) {
	rlimits (0; -1) {
	    mixed *callout;
	    int i;

	    suspender = nil;
	    suspend = 1;
	    if (first_suspended) {
		/* start releasing callouts */
		callout = first_suspended;
		do {
		    if (callout[CO_RELHANDLE] == 0) {
			callout[CO_RELHANDLE] = call_out("release_callout", 0);
		    }
		    callout = callout[CO_NEXT];
		} while (callout);
	    } else {
		/* no callouts to release */
		suspended = nil;
		suspend = 0;
		rsrcd->resume_callouts();
	    }

	    /* unblock input on connections */
	    connections -= ({ nil });
	    for (i = sizeof(connections); --i >= 0; ) {
		connections[i]->set_mode(MODE_UNBLOCK);
	    }
	    connections = nil;

	    /* release user */
	    if (user) {
		user->release();
	    }
	}
    }
}

/*
 * NAME:	suspend_callout()
 * DESCRIPTION:	suspend a callout
 */
void suspend_callout(object obj, int handle)
{
    if (previous_object() == rsrcd) {
	mixed *callout;

	callout = ({ obj, handle, 0, last_suspended, nil });
	if (suspend > 0) {
	    callout[CO_RELHANDLE] = call_out("release_callout", 0);
	}
	if (last_suspended) {
	    last_suspended[CO_NEXT] = callout;
	} else {
	    first_suspended = callout;
	}
	last_suspended = callout;
	if (suspended[obj]) {
	    suspended[obj][handle] = callout;
	} else {
	    suspended[obj] = ([ handle : callout ]);
	}
    }
}

/*
 * NAME:	remove_callout()
 * DESCRIPTION:	remove callout from list of suspended calls
 */
int remove_callout(mixed tls, object obj, int handle)
{
    mapping callouts;
    mixed *callout;

    if (previous_object() == rsrcd && (callouts=suspended[obj]) &&
	(callout=callouts[handle])) {
	if (callout != first_suspended) {
	    callout[CO_PREV][CO_NEXT] = callout[CO_NEXT];
	} else {
	    first_suspended = callout[CO_NEXT];
	}
	if (callout != last_suspended) {
	    if (callout[CO_RELHANDLE] != 0) {
		remove_call_out(last_suspended[CO_RELHANDLE]);
		last_suspended[CO_RELHANDLE] = callout[CO_RELHANDLE];
	    }
	    callout[CO_NEXT][CO_PREV] = callout[CO_PREV];
	} else {
	    if (callout[CO_RELHANDLE] != 0) {
		remove_call_out(callout[CO_RELHANDLE]);
	    }
	    last_suspended = callout[CO_PREV];
	}
	callouts[handle] = nil;
	return TRUE;	/* delayed call */
    }
    return FALSE;
}

/*
 * NAME:	remove_callouts()
 * DESCRIPTION:	remove callouts from an object about to be destructed
 */
void remove_callouts(object obj)
{
    if (previous_object() == rsrcd && suspended[obj]) {
	mixed **callouts, *callout;
	int i;

	callouts = map_values(suspended[obj]);
	for (i = sizeof(callouts); --i >= 0; ) {
	    callout = callouts[i];
	    if (callout != first_suspended) {
		callout[CO_PREV][CO_NEXT] = callout[CO_NEXT];
	    } else {
		first_suspended = callout[CO_NEXT];
	    }
	    if (callout != last_suspended) {
		if (callout[CO_RELHANDLE] != 0) {
		    remove_call_out(last_suspended[CO_RELHANDLE]);
		    last_suspended[CO_RELHANDLE] = callout[CO_RELHANDLE];
		}
		callout[CO_NEXT][CO_PREV] = callout[CO_PREV];
	    } else {
		if (callout[CO_RELHANDLE] != 0) {
		    remove_call_out(callout[CO_RELHANDLE]);
		}
		last_suspended = callout[CO_PREV];
	    }
	}
	suspended[obj] = nil;
    }
}

/*
 * NAME:	release_callout()
 * DESCRIPTION:	release a callout
 */
static void release_callout()
{
    if (suspend >= 0) {
	mixed *callout;
	object obj;
	int handle;

	if (first_suspended) {
	    callout = first_suspended;
	    obj = callout[CO_OBJ];
	    handle = callout[CO_HANDLE];
	    if ((first_suspended=callout[CO_NEXT])) {
		first_suspended[CO_PREV] = nil;
		suspended[obj][handle] = nil;
	    } else {
		last_suspended = nil;
		suspended = nil;
		suspend = 0;
		call_out("release_callout", 0);
	    }

	    rsrcd->release_callout(obj, handle);
	} else {
	    rsrcd->resume_callouts();
	}

    }
}

/*
 * NAME:	suspend_connection()
 * DESCRIPTION:	suspend a new connection
 */
void suspend_connection(object conn)
{
    if (SYSTEM()) {
	conn->set_mode(MODE_BLOCK);
	connections = connections - ({ nil }) + ({ conn });
    }
}

/*
 * NAME:	query_suspended()
 * DESCRIPTION:	return TRUE if the system is suspended, FALSE otherwise
 */
int query_suspended()
{
    return (suspend < 0);
}


/*
 * NAME:	prepare_reboot()
 * DESCRIPTION:	prevent state dump creation if suspended
 */
void prepare_reboot()
{
    if (suspend < 0) {
	error("Cannot dump state while suspended");
    }
}


/*
 * NAME:	task()
 * DESCRIPTION:	allow suspender to start a delayed call
 */
void task(string func, mixed args...)
{
    if (previous_object() == suspender) {
	call_out("callback", 0, func, args);
    }
}

/*
 * NAME:	callback()
 * DESCRIPTION:	call function in suspender
 */
static void callback(string func, mixed *args)
{
    call_other(suspender, func, args...);
}
