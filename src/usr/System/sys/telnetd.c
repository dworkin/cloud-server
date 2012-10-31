# include <kernel/kernel.h>
# include <kernel/user.h>
# include <systemd.h>

inherit "~/lib/auto";
inherit LIB_USER;

# define PLAYERD	"/usr/Player/sys/userd"
# define WIZTOOL	"/usr/System/obj/wiztool"


object userd;		/* user daemon */
object systemd;		/* system daemon */
string banner;		/* login message */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize telnet daemon
 */
static create()
{
    userd = find_object(USERD);
    userd->set_telnet_manager(0, this_object());
    systemd = find_object(SYSTEMD);

    banner = "\n" +
	     "Welcome to the Cloud Server.\n" +
	     "\n" +
	     "After login, the following commands are available:\n" +
	     " - @users               see who is logged on\n" +
	     " - say, ', emote, :     communication\n" +
	     " - @quit                leave the mud\n" +
	     "\n" +
	     "login: ";
}

/*
 * NAME:	select()
 * DESCRIPTION:	select user object based on loginname
 */
object select(string name)
{
    if (previous_object() == userd) {
	object obj;

	obj = PLAYERD->get_user(name);
	return (obj) ? obj : this_object();
    }
}

/*
 * NAME:	query_timeout()
 * DESCRIPTION:	return the login timeout
 */
int query_timeout(object obj)
{
    if (previous_object() == userd && systemd->query_suspended()) {
	systemd->suspend_connection(obj);
    }
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
    previous_object()->message("\"" + str + "\" is not a valid name.\n\n");
    return MODE_DISCONNECT;
}


/*
 * NAME:	add_wiztool()
 * DESCRIPTION:	give the user a wiztool
 */
void add_wiztool(object user)
{
    if (SYSTEM()) {
	user->add_wiztool(clone_object(WIZTOOL, user->query_name()));
    }
}

/*
 * NAME:	remove_wiztool()
 * DESCRIPTION:	remove the user's wiztool
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
