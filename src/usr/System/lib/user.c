# include <kernel/kernel.h>
# include <kernel/user.h>

inherit "auto";
inherit user LIB_USER;


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

    return user::query_ip_name(obj);
}
