# include <cmdparser.h>

private inherit "/lib/util/language";


private object parser;	/* central command parser */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize parser handling
 */
static void create()
{
    parser = find_object("/sys/cmdparser");
}

/*
 * NAME:	parse()
 * DESCRIPTION:	parse a command
 */
static mixed **parse(string str)
{
    return parser->parse(str);
}

static string match_string(mixed *things, string *strs)
{
    strs &= ({ things[0][THING_NOUN] });
    if (sizeof(strs) != 0) {
	return strs[0];
    }
    return nil;
}

private int match_one_object(mapping name_cache, mapping plname_cache,
			     mapping plural_cache, mapping adj_cache,
			     mixed **things, int *counters, object obj)
{
    string *names, *plnames, *adjectives, *adjs, noun, str, plstr;
    int i, j, plural, matches;
    mixed *thing;

    names = name_cache[obj];
    if (!names) {
	names = name_cache[obj] = obj->query_names();
	plural = plural_cache[obj] = obj->query_plural();
    } else {
	plural = plural_cache[obj];
    }

    matches = 0;
    for (i = sizeof(things); --i >= 0; ) {
	thing = things[i];

	/* names */
	noun = thing[THING_NOUN];
	if (plural) {
	    /*
	     * object is plural
	     */
	    if (thing[THING_PFLAGS] & PARSER_PLURAL) {
		/* thing is plural */
		if (sizeof(names & ({ noun })) == 0) {
		    continue;	/* no match */
		}
	    } else {
		/* thing is singular or unknown */
		if ((thing[THING_PFLAGS] & PARSER_SINGULAR) ||
		    sizeof(names & ({ noun })) == 0) {
		    plstr = plname_cache[noun];
		    if (!plstr) {
			plstr = plname_cache[noun] = pluralize(noun);
		    }

		    /* thing is singular or does not match plural object */
		    if (sizeof(names & ({ plstr })) == 0) {
			continue;	/* no match */
		    }
		    /* XXX match plural obj with singular thing */
		}
	    }
	} else {
	    /*
	     * object is singular
	     */
	    if (thing[THING_PFLAGS] & PARSER_SINGULAR) {
		/* thing is singular */
		if (sizeof(names & ({ noun })) == 0) {
		    continue;	/* no match */
		}
	    } else {
		/* thing is unknown or plural */
		if ((thing[THING_PFLAGS] & PARSER_PLURAL) ||
		    sizeof(names & ({ noun })) == 0) {
		    plnames = plname_cache[obj];
		    if (!plnames) {
			plnames = plname_cache[obj] = allocate(sizeof(names));
			for (j = sizeof(plnames); --j >= 0; ) {
			    str = names[j];
			    plstr = plname_cache[str];
			    if (!plstr) {
				plstr = plname_cache[str] = pluralize(str);
			    }
			    plnames[j] = plstr;
			}
		    }

		    if (sizeof(plnames & ({ noun })) == 0) {
			continue;	/* no match */
		    }
		}
	    }
	}

	/* adjectives */
	adjectives = thing[THING_ADJECTIVES];
	if (sizeof(adjectives) != 0) {
	    adjs = adj_cache[obj];
	    if (!adjs) {
		adjs = adj_cache[obj] = obj->query_adjs();
	    }
	    if (sizeof(adjectives & adjs) != sizeof(adjectives)) {
		continue;	/* no match */
	    }
	}

	/* XXX check owner, aspect, title */

	counters[i]++;
	if (thing[THING_PFLAGS] & PARSER_COUNT) {
	    if (counters[i] <= thing[THING_NUM]) {
		matches++;
	    }
	} else if (thing[THING_PFLAGS] & PARSER_CARDINAL) {
	    if (counters[i] == thing[THING_NUM]) {
		matches++;
	    }
	} else {
	    matches++;
	}
    }

    return matches;
}

static object match_objects(mixed *things, object *objs)
{
    string str;
    int i, sz, *counters;

    counters = allocate_int(sizeof(things));
    for (i = 0, sz = sizeof(objs); i < sz; i++) {
	if (match_one_object(([ ]), ([ ]), ([ ]), ([ ]), things, counters, objs[i])) {
	    return objs[i];
	}
    }
    return nil;
}

static mixed *match_detail(mixed *things, object *objs)
{
    string str;
    int i, sz;

    str = things[0][THING_NOUN];
    for (i = 0, sz = sizeof(objs); i < sz; i++) {
	if (objs[i]->query_detail_desc(str)) {
	    return ({ objs[i], str });
	}
    }
}
