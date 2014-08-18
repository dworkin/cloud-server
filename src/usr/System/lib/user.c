# include <kernel/kernel.h>
# include <kernel/user.h>

inherit LIB_USER;


/*
 * NAME:	set_mode()
 * DESCRIPTION:	set the connection mode
 */
static void set_mode(int mode)
{
    query_conn()->set_mode(mode);
}

/*
 * NAME:	login()
 * DESCRIPTION:	login this user
 */
static void login(string str)
{
    DRIVER->message(str);
}

/*
 * NAME:	logout()
 * DESCRIPTION:	logout this user
 */
static void logout()
{
}

/*
 * NAME:	address()
 * DESCRIPTION:	return the address this object is connected to
 */
static string address()
{
    object obj;

    obj = query_conn();
    while (obj <- LIB_USER) {
	obj = obj->query_conn();
    }

    return ::query_ip_name(obj);
}
