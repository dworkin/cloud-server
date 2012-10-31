inherit body	"/lib/base/body";
inherit inv	"/lib/container";

private inherit "/lib/util/string";
private inherit "/lib/util/language";


private string race;		/* race of this body */
private string actor;		/* this body, as it shows up in actions */


/*
 * NAME:	create()
 * DESCRIPTION:	initialize body
 */
static void create(string str)
{
    body::create();
    inv::create();

    ::set_name(race = str);
}

/*
 * NAME:	query_short_desc()
 * DESCRIPTION:	body override of short description
 */
string query_short_desc()
{
    string str;

    str = ::query_short_desc();
    return (str) ? str : query_name();
}

/*
 * NAME:	query_long_desc()
 * DESCRIPTION:	body override of long description
 */
string query_long_desc()
{
    string str;

    str = ::query_long_desc();
    if (!str) {
	str = query_short_desc();
	if (str) {
	    if (query_plural()) {
		return str;
	    }
	    if (!query_unique()) {
		str = article(str) + " " + str;
	    }
	}
    }
    return str;
}

/*
 * NAME:	query_race()
 * DESCRIPTION:	return the race of this body
 */
string query_race()
{
    return race;
}

/*
 * NAME:	set_actor_desc()
 * DESCRIPTION:	set the actor description
 */
void set_actor_desc(string str)
{
    actor = str;
}

/*
 * NAME:	query_actor_desc()
 * DESCRIPTION:	return the actor
 */
string query_actor_desc()
{
    return (actor) ? actor : query_short_desc();
}

/*
 * NAME:	set_name()
 * DESCRIPTION:	give this body a name
 */
void set_name(string names...)
{
    ::set_name(names | ({ race })...);
    if (!::query_short_desc()) {
	set_short_desc(capitalize(names[0]));
    }
    set_unique(1);
}

/*
 * NAME:	message()
 * DESCRIPTION:	pass on a message to the soul, if there is one
 */
void message(int type, string str, object skip...)
{
    if (query_soul()) {
	query_soul()->message(type, str);
    }
}

/*
 * NAME:	command()
 * DESCRIPTION:	handle an action
 */
static mixed command(string str, mixed *args)
{
    return call_other(this_object(), "act_" + str, args...);
}

/*
 * query functions that define this body's view of the surrounding space
 */
object  query_act_env()		{ return query_env(); }
object *query_act_objects()	{ return query_env()->query_inv(); }
