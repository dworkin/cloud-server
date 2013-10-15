# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/user.h>
# include <kernel/rsrc.h>

inherit access API_ACCESS;
inherit rsrc API_RSRC;

# define BinaryServer	"/usr/WWW/sys/server"
# define TelnetServer	"/usr/Player/sys/userd"
# define WizTool	"/usr/System/obj/wiztool"


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

    /* clonables */
    compile_object("obj/wiztool");

    /* mudlib objects */
    compile_object("/obj/body/human");
    compile_object("/data/strbuffer");

    /* Domain stuff */
    rsrc_incr(nil, "filequota", nil,
	      DRIVER->file_size("/lib", TRUE) +
	      DRIVER->file_size("/obj", TRUE), TRUE);
    domains = get_dir("/usr/[A-Z]*")[0];
    for (i = 0, sz = sizeof(domains); i < sz; i++) {
	domain = domains[i];
	if (domain != "System" && file_info("/usr/" + domain + "/initd.c")) {
	    add_owner(domain);
	    rsrc_incr(domain, "filequota", nil,
		      DRIVER->file_size("/usr/" + domain, TRUE), TRUE);
	    load("/usr/" + domain + "/initd");
	}
    }

    /* connections */
    USERD->set_binary_manager(0, find_object(BinaryServer));
    USERD->set_telnet_manager(0, find_object(TelnetServer));
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

	rsrc_incr(nil, "filequota", nil,
		  DRIVER->file_size("/doc", TRUE) +
		  DRIVER->file_size("/include", TRUE) +
		  DRIVER->file_size("/lib", TRUE) +
		  DRIVER->file_size("/obj", TRUE) +
		  DRIVER->file_size("/sys", TRUE) -
		  rsrc_get(nil, "filequota")[RSRC_USAGE],
		  TRUE);
	owners = query_owners();
	for (i = 1, sz = sizeof(owners); i < sz; i++) {
	    rsrc_incr(owners[i], "filequota", nil,
		      DRIVER->file_size("/usr/" + owners[i], TRUE) -
		      rsrc_get(owners[i], "filequota")[RSRC_USAGE],
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
	 user->query_name() == "admin" && user->query_conn())) {
	user->add_wiztool(clone_object(WizTool, user->query_name()));
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
