# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/user.h>
# include <version.h>

inherit kernel AUTO;
inherit "~/lib/auto";
inherit "~/lib/user";
inherit access API_ACCESS;

private inherit "/lib/util/string";
private inherit "/lib/util/version";

# define USER		"/usr/System/obj/user"

object userd;		/* kernel user daemon */
mapping users;		/* mapping for user objects */
string banner;		/* login message */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize this object
 */
static void create(void)
{
    access::create();
    userd = find_object(USERD);
    userd->set_telnet_manager(0, this_object());
    users = ([ ]);
    banner = "\n" +
	     "Welcome to " + version() + ".\n" +
	     "\n" +
	     "After login, the following commands are available:\n" +
	     " - users                see who is logged on\n" +
	     " - say, ', emote, :     communication\n" +
	     " - quit                 leave the mud\n" +
	     "\n" +
	     "login: ";
}

/*
 * NAME:	find_user()
 * DESCRIPTION:	find a user object
 */
object find_user(string name)
{
    return users[lower_case(name)];
}

/*
 * NAME:	get_user()
 * DESCRIPTION:	return the proper user object, creating a new one if required
 */
static object get_user(string name)
{
    object obj;

    if (!parse_string("word = /[A-Za-z][A-Za-z]+/ junk = /./ name: word",
		      name)) {
	/* bad name */
	return nil;
    }

    name = lower_case(name);
    if (sizeof(access::query_users() & ({ name })) == 0) {
	return nil;
    }

    obj = users[name];
    if (!obj) {
	obj = users[name] = kernel::clone_object("~/obj/user", name);
    }
    return obj;
}

/*
 * NAME:	select()
 * DESCRIPTION:	select user object based on loginname
 */
object select(string name)
{
    if (previous_object() == userd) {
	object obj;

	obj = get_user(name);
	return (obj) ? obj : this_object();
    }
}

/*
 * NAME:	query_timeout()
 * DESCRIPTION:	return the login timeout
 */
int query_timeout(object obj)
{
    return DEFAULT_TIMEOUT;
}

/*
 * NAME:	set_banner()
 * DESCRIPTION:	set the login banner
 */
void set_banner(string str)
{
    if (SYSTEM()) {
	banner = str;
    }
}

/*
 * NAME:	query_banner()
 * DESCRIPTION:	return the login banner
 */
string query_banner(object obj)
{
    return banner;
}

/*
 * NAME:	login()
 * DESCRIPTION:	display an errormessage and disconnect
 */
int login(string str)
{
    previous_object()->message("Access denied.\n\n");
    return MODE_DISCONNECT;
}
