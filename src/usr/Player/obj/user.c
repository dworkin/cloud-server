# include <config.h>
# include <kernel/user.h>

inherit telnet	"~/lib/interface/telnet";


int last_activity;	/* time of last activity */

/*
 * NAME:	login()
 * DESCRIPTION:	pass on call to the correct interface
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	last_activity = time();
	return telnet::login(str);
    }
}

/*
 * NAME:	logout()
 * DESCRIPTION:	pass on call to the correct interface
 */
void logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	if (quit) {
	    last_activity = time();
	}
	telnet::logout(quit);
    }
}

/*
 * NAME:	message()
 * DESCRIPTION:	pass on call to the correct interface
 */
void message(int type, string str)
{
    telnet::message(type, str);
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	pass on call to the correct interface
 */
int receive_message(string str)
{
    if (previous_program() == LIB_CONN) {
	last_activity = time();
	return telnet::receive_message(str);
    }
}

/*
 * NAME:	query_last_activity()
 * DESCRIPTION:	return time of last activity
 */
int query_last_activity()
{
    return last_activity;
}
