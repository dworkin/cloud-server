private object userd;	/* user daemon */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize this object
 */
static void create()
{
    userd = find_object("~/sys/userd");
}

/*
 * NAME:	find_user()
 * DESCRIPTION:	find the user object of a player
 */
static object find_user(string name)
{
    return userd->find_user(name);
}
