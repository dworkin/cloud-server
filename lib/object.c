inherit movable	"/lib/base/movable";
inherit desc	"/lib/base/description";


private string *names;		/* object names */
private string *adjectives;	/* object adjectives */
private string plshort_desc;	/* plural short description, if any */
private int flags;		/* object flags */

# define F_UNIQUE	0x01	/* object is unique */
# define F_PLURAL	0x02	/* object is plural */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize object
 */
static void create()
{
    if (!names) {
	movable::create();
	desc::create();

	names = ({ });
    }
}

/*
 * NAME:	set_name()
 * DESCRIPTION:	set all the names of the object
 */
void set_name(string namelist...)
{
    names = namelist - ({ nil });
}

/*
 * NAME:	set_adj()
 * DESCRIPTION:	set all the adjectives of this object
 */
void set_adj(string adjlist...)
{
    adjectives = adjlist - ({ nil });
}

/*
 * NAME:	add_name()
 * DESCRIPTION:	add an extra name to the object
 */
void add_name(string name)
{
    if (name) {
	names |= ({ name });
    }
}

/*
 * NAME:	add_adj()
 * DESCRIPTION:	add an adjective to this object
 */
void add_adj(string adj)
{
    if (adj) {
	if (adjectives) {
	    adjectives |= ({ adj });
	} else {
	    adjectives = ({ adj });
	}
    }
}

/*
 * NAME:	remove_name()
 * DESCRIPTION:	remove one of the object's names
 */
void remove_name(string name)
{
    names -= ({ name });
}

/*
 * NAME:	remove_adj()
 * DESCRIPTION:	remove an adjective from this object
 */
void remove_adj(string adj)
{
    if (adjectives) {
	adjectives -= ({ adj });
    }
}

/*
 * NAME:	query_name()
 * DESCRIPTION:	return the first, primary name of this object
 */
string query_name()
{
    return (sizeof(names) != 0) ? names[0] : nil;
}

/*
 * NAME:	query_names()
 * DESCRIPTION:	return the list of names of this object
 */
string *query_names()
{
    return names[..];
}


/*
 * NAME:	query_adjs()
 * DESCRIPTION:	return the list of adjectives of this object
 */
string *query_adjs()
{
    return (adjectives) ? adjectives[..] : ({ });
}

/*
 * NAME:	id()
 * DESCRIPTION:	Return FALSE or TRUE depending on whether the description
 *		matches this object.  This function should almost never
 *		be called.
 */
int id(string name, string adj...)
{
    if (sizeof(names & ({ name })) == 0) {
	return FALSE;
    }
    if (adjectives) {
	if (sizeof(adj & adjectives) != sizeof(adj)) {
	    return FALSE;
	}
    } else if (sizeof(adj) != 0) {
	return FALSE;
    }
    return TRUE;
}


/*
 * NAME:	set_unique()
 * DESCRIPTION:	object is unique ('the' rather than 'a' in descriptions)
 */
void set_unique(int unique)
{
    if (unique) {
	flags |= F_UNIQUE;
    } else {
	flags &= ~F_UNIQUE;
    }
}

/*
 * NAME:	set_plural()
 * DESCRIPTION:	set the plural flag
 */
void set_plural(int plural)
{
    if (plural) {
	flags |= F_PLURAL;
    } else {
	flags &= ~F_PLURAL;
    }
}

/*
 * NAME:	set_plural_short_desc()
 * DESCRIPTION:	set the plural short description
 */
void set_plural_short_desc(string str)
{
    plshort_desc = str;
}

/*
 * NAME:	query_unique()
 * DESCRIPTION:	return the unique flag
 */
int query_unique()
{
    return !!(flags & F_UNIQUE);
}

/*
 * NAME:	query_plural()
 * DESCRIPTION:	return the plural flag
 */
int query_plural()
{
    return !!(flags & F_PLURAL);
}

/*
 * NAME:	query_plural_short_desc()
 * DESCRIPTION:	return the plural short description
 */
string query_plural_short_desc()
{
    return plshort_desc;
}


/*
 * NAME:	leave_env()
 * DESCRIPTION:	function called when object is leaving its environment
 */
static void leave_env(object from, object to)
{
}

/*
 * NAME:	enter_env()
 * DESCRIPTION:	function called when object has entered a new environment
 */
static void enter_env(object to, object from)
{
}

/*
 * NAME:	move()
 * DESCRIPTION:	move this object to a new destination, return 1 on success
 *		or 0 on failure
 */
int move(object dest, varargs mixed position)
{
    object env;

    env = query_env();

    /* XXX add can-move checks here */

    if (env == dest) {
	/* move to a different position in the same environment */
	::move(dest, position);
    } else {
	if (env) {
	    leave_env(env, dest);
	    env->leave_inv(this_object(), dest);
	}

	::move(dest, position);

	if (dest) {
	    enter_env(dest, env);
	    dest->enter_inv(this_object(), env);
	}
    }

    return 1;
}
