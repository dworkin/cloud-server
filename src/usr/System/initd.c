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

    /* global objects */
    compile_object("/sys/utf8encode");
    compile_object("/sys/utf8decode");
    compile_object("/lib/Iterator");
    compile_object("/lib/String");
    compile_object("/lib/StringBuffer");

    /* Domain stuff */
    rsrc_incr(nil, "fileblocks",
	      DRIVER->file_size("/lib", TRUE) +
	      DRIVER->file_size("/obj", TRUE), TRUE);
    domains = get_dir("/usr/[A-Z]*")[0];
    for (i = 0, sz = sizeof(domains); i < sz; i++) {
	domain = domains[i];
	if (domain != "System" && file_info("/usr/" + domain + "/initd.c")) {
	    add_owner(domain);
	    rsrc_incr(domain, "fileblocks",
		      DRIVER->file_size("/usr/" + domain, TRUE), TRUE);
	    load("/usr/" + domain + "/initd");
	}
    }

    /* clonables */
    compile_object("obj/user");

    /* connections */
    USERD->set_binary_manager(0, find_object(BINARYSERVER));
    USERD->set_telnet_manager(0, find_object(TELNETSERVER));
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
		  rsrc_get(nil, "fileblocks")[RSRC_USAGE],
		  TRUE);
	owners = query_owners();
	for (i = 1, sz = sizeof(owners); i < sz; i++) {
	    rsrc_incr(owners[i], "fileblocks",
		      DRIVER->file_size("/usr/" + owners[i], TRUE) -
		      rsrc_get(owners[i], "fileblocks")[RSRC_USAGE],
		      TRUE);
	}
    }
}


/*
 * NAME:	add_wiztool()
 * DESCRIPTION:	give user a wiztool
 */
void add_wiztool(object user)
{
    if (SYSTEM() ||
	(previous_program() == "/usr/Player/lib/interface" &&
	 user->query_conn() &&
	 (rsrc::query_owners() & ({ user->query_name() })))) {
	user->add_wiztool(clone_object(WIZTOOL, user->query_name()));
    }
}

/*
 * NAME:	remove_wiztool()
 * DESCRIPTION:	remove user's wiztool
 */
void remove_wiztool(object user)
{
    if (SYSTEM()) {
	object wiztool;

	wiztool = user->query_wiztool();
	user->remove_wiztool();
	destruct_object(wiztool);
    }
}
