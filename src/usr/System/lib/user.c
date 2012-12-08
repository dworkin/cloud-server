# include <kernel/kernel.h>
# include <kernel/user.h>

inherit LIB_USER;

# define Init	"/usr/System/initd"


static void log_connection(string str)
{
    DRIVER->message(str);
}

static nomask string connected()
{
    object obj;

    obj = query_conn();
    while (obj <- LIB_USER) {
	obj = obj->query_conn();
    }

    return ::query_ip_name(obj);
}

static void admin_wiztool()
{
    if (connected()) {
	Init->add_wiztool(this_object());
    }
}
