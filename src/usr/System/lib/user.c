# include <kernel/kernel.h>
# include <kernel/user.h>

inherit LIB_USER;

# define Init	"/usr/System/initd"


/*
 * NAME:	set_mode()
 * DESCRIPTION:	set the connection mode
 */
static void set_mode(int mode)
{
    query_conn()->set_mode(mode);
}

/*
 * NAME:	log_connection()
 * DESCRIPTION:	log this connection
 */
static void log_connection(string str)
{
    DRIVER->message(str);
}

/*
 * NAME:	connected()
 * DESCRIPTION:	return the address this object is connected to
 */
static nomask string connected()
{
    object obj;

    obj = query_conn();
    while (obj <- LIB_USER) {
	obj = obj->query_conn();
    }

    return ::query_ip_name(obj);
}

/*
 * NAME:	admin_wiztool()
 * DESCRIPTION:	create a wiztool when the admin first logs in
 */
static void admin_wiztool()
{
    if (connected()) {
	Init->add_wiztool(this_object());
    }
}
