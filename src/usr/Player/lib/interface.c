# include <kernel/user.h>

inherit user "~System/lib/user";
inherit soul "/lib/base/soul";

private inherit "/lib/util/string";

# define SystemInit	"/usr/System/initd"


private object body;		/* current body */
private object wiztool;		/* wiztool, if any */

/*
 * NAME:	login()
 * DESCRIPTION:	establish link with connection object and login user
 */
static void login(object connection, string name)
{
    ::connection(connection);
    ::login(name);
    log_connection("Connection: " + capitalize(name) + " from " + address() +
		   "\n");
    if (name == "admin" && !wiztool) {
	SystemInit->add_wiztool(this_object());
    }
}

/*
 * NAME:	logout()
 * DESCRIPTION:	logout user
 */
static void logout()
{
    if (query_conn()) {
	::logout();
    }
}

/*
 * NAME:	set_current_body()
 * DESCRIPTION:	this body currently has the user's focus
 */
static void set_current_body(object obj)
{
    body = obj;
}

/*
 * NAME:	add_wiztool()
 * DESCRIPTION:	give the user a wiztool
 */
void add_wiztool(object obj)
{
    if (previous_program() == SystemInit) {
	add_body(wiztool = obj);
    }
}

/*
 * NAME:	remove_wiztool()
 * DESCRIPTION:	remove the user's wiztool
 */
void remove_wiztool()
{
    if (previous_program() == SystemInit) {
	remove_body(wiztool);
	wiztool = nil;
    }
}

object query_body()	{ return body; }
object query_wiztool()	{ return wiztool; }
