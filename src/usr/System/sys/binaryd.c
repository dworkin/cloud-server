# include <kernel/kernel.h>
# include <kernel/user.h>

inherit LIB_USER;

# define PLAYERD	"/usr/Player/sys/userd"


object userd;		/* user daemon */
string errormessage;	/* message returned in case of a login error */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize binary connection daemon
 */
static create()
{
    userd = find_object(USERD);
    userd->set_binary_manager(0, this_object());

    errormessage = "<HTML>\n" +
		   "<HEAD><TITLE>400 Bad Request</TITLE></HEAD>\n" +
		   "<BODY><H1>400 Bad Request</H1></BODY>\n" +
		   "</HTML>\n";
}

/*
 * NAME:	select()
 * DESCRIPTION:	select user object based on loginname
 */
object select(string str)
{
    if (previous_object() == userd) {
	object obj;

	obj = "~WWW/sys/server"->request(str);
	return (obj) ? obj : this_object();
    }
}

/*
 * NAME:	query_timeout()
 * DESCRIPTION:	return login timeout
 */
int query_timeout(object obj)
{
    return 30;
}

/*
 * NAME:	query_banner()
 * DESCRIPTION:	return login banner
 */
string query_banner(object obj)
{
    return nil;
}

/*
 * NAME:	set_errormessage()
 * DESCRIPTION:	set the errormessage shown in case of a login error
 */
void set_errormessage(string str)
{
    if (SYSTEM()) {
	errormessage = str;
    }
}

/*
 * NAME:	login()
 * DESCRIPTION:	display an errormessage and disconnect
 */
int login(string str)
{
    previous_object()->message(errormessage);
    return MODE_DISCONNECT;
}
