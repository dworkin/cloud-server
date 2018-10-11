# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/user.h>
# include <kernel/rsrc.h>

inherit access API_ACCESS;
inherit rsrc API_RSRC;

# define BINARYSERVER	"/usr/WWW/sys/server"
# define TELNETSERVER	"/usr/System/sys/userd"
# define WIZTOOL	"/usr/System/obj/wiztool"


/*
 * NAME:	load()
 * DESCRIPTION:	compile and initialize an object
 */
private void load(string path)
{
    call_other(compile_object(path), "???");
}

/*
 * NAME:	create()
 * DESCRIPTION:	initialize the system
 */
static void create()
{
    string *domains;
    string domain;
    int i, sz;

    access::create();
    rsrc::create();

    /* object registry */
    load("sys/objectd");

    /* global access */
    set_global_access("System", TRUE);

    /* server objects */
    load("sys/errord");
    load("sys/upgraded");
    load("sys/userd");

    /* clonables */
    compile_object("obj/user");

    /* global objects */
    compile_object("/sys/utf8encode");
    compile_object("/sys/utf8decode");
    compile_object("/lib/IntIterator");
    compile_object("/lib/String");
    compile_object("/lib/StringBuffer");
    compile_object("/lib/GMTime");
    compile_object("/lib/ChainedContinuation");
    compile_object("/lib/DelayedContinuation");
    compile_object("/lib/IterativeContinuation");
    compile_object("/lib/DistContinuation");
    compile_object("/lib/ContinuationToken");

    /* Domain stuff */
    rsrc_incr(nil, "fileblocks",
	      DRIVER->file_size("/lib", TRUE) +
	      DRIVER->file_size("/obj", TRUE));
    domains = get_dir("/usr/[A-Z]*")[0];
    for (i = 0, sz = sizeof(domains); i < sz; i++) {
	domain = domains[i];
	if (domain != "System" && file_info("/usr/" + domain + "/initd.c")) {
	    add_owner(domain);
	    rsrc_incr(domain, "fileblocks",
		      DRIVER->file_size("/usr/" + domain, TRUE));
	    rlimits (100; 1000000) {
		load("/usr/" + domain + "/initd");
	    }
	}
    }
}

/*
 * NAME:	prepare_reboot()
 * DESCRIPTION:	called from the driver object before a snapshot is made
 */
void prepare_reboot()
{
    if (previous_program() == DRIVER) {
    }
}

/*
 * NAME:	reboot()
 * DESCRIPTION:	get file quotas right after a reboot
 */
void reboot()
{
    if (previous_program() == DRIVER) {
	string *owners;
	int i, sz;

	rsrc_incr(nil, "fileblocks",
		  DRIVER->file_size("/doc", TRUE) +
		  DRIVER->file_size("/include", TRUE) +
		  DRIVER->file_size("/lib", TRUE) +
		  DRIVER->file_size("/obj", TRUE) +
		  DRIVER->file_size("/sys", TRUE) -
		  rsrc_get(nil, "fileblocks")[RSRC_USAGE]);
	owners = query_owners();
	for (i = 1, sz = sizeof(owners); i < sz; i++) {
	    rsrc_incr(owners[i], "fileblocks",
		      DRIVER->file_size("/usr/" + owners[i], TRUE) -
		      rsrc_get(owners[i], "fileblocks")[RSRC_USAGE]);
	}
    }
}

/*
 * NAMR:	set_connection_manager()
 * DESCRIPRION:	first come, first go
 */
void set_connection_manager(string type, int port, object manager)
{
    call_other(USERD, "set_" + type + "_manager", port, manager);
}
