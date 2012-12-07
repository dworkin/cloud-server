# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/rsrc.h>

inherit access API_ACCESS;
inherit rsrc API_RSRC;


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
    compile_object("sys/objectd");

    /* global access */
    set_global_access("System", TRUE);

    DRIVER->set_tls_size(2);

    /* server objects */
    compile_object("sys/errord");
    compile_object("sys/telnetd");
    compile_object("sys/binaryd");
    compile_object("sys/upgraded");

    /* clonables */
    compile_object("obj/wiztool");

    /* mudlib objects */
    compile_object("/sys/cmdparser");
    compile_object("/sys/sould");
    compile_object("/obj/body/human");
    compile_object("/data/strbuffer");

    /* Domain stuff */
    rsrc_incr(nil, "filequota", nil,
	      DRIVER->file_size("/lib", TRUE) +
	      DRIVER->file_size("/obj", TRUE) +
	      DRIVER->file_size("/sys", TRUE), TRUE);
    domains = get_dir("/usr/[A-Z]*")[0];
    for (i = 0, sz = sizeof(domains); i < sz; i++) {
	domain = domains[i];
	if (domain != "System" && file_info("/usr/" + domain + "/initd.c")) {
	    add_owner(domain);
	    rsrc_incr(domain, "filequota", nil,
		      DRIVER->file_size("/usr/" + domain, TRUE), TRUE);
	    compile_object("/usr/" + domain + "/initd");
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
