# include <kernel/kernel.h>
# include <kernel/rsrc.h>
# include <type.h>
# include <status.h>
# include <trace.h>

# define TLS()		call_trace(1)[TRACE_FIRSTARG]


object rsrcd;		/* resource manager */
mapping resources;	/* registered resources */
string owner;		/* owner of these resources */
int maxticks;		/* maximum number of ticks currently allowed */
float maxusage;		/* maximum tick usage */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize resource mapping
 */
static void create()
{
    resources = ([
      "stack" :		({   0, -1, 0 }),
      "ticks" :		({ 0.0, -1, 0 })
    ]);
    maxticks = -1;
    maxusage = -1.0;
    rsrcd = find_object(RSRCD);
}

/*
 * NAME:	init()
 * DESCRIPTION:	initialize resource
 */
void init(string name, int time)
{
    if (previous_object() == rsrcd) {
	owner = name;
	resources["ticks"][RSRC_DECAYTIME] = time;
    }
}

/*
 * NAME:	remove_rsrc()
 * DESCRIPTION:	remove a resource
 */
void remove_rsrc(string name)
{
    if (previous_object() == rsrcd) {
	resources[name] = nil;
    }
}

/*
 * NAME:	decay_rsrc()
 * DESCRIPTION:	decay a resource
 */
private void decay_rsrc(mixed *rsrc, int *grsrc, int time)
{
    float usage, decay;
    int period, t;

    usage = rsrc[RSRC_USAGE];
    decay = (float) (100 - grsrc[GRSRC_DECAY]) / 100.0;
    period = grsrc[GRSRC_PERIOD];

    t = (time - (int) rsrc[RSRC_DECAYTIME]) / period;
    usage *= pow(decay, (float) t);

    if (usage < 0.5) {
	rsrc[RSRC_DECAYTIME] = time;
    } else {
	rsrc[RSRC_DECAYTIME] += t * period;
    }
    rsrc[RSRC_USAGE] = floor(usage + 0.5);
}

/*
 * NAME:	set_rlimits()
 * DESCRIPTION:	set stack/tick limits
 */
private void set_rlimits(mixed *rsrc, int update)
{
    int max;

    max = (maxusage > 0.0 && rsrc[RSRC_USAGE] >= maxusage) ? 1 : rsrc[RSRC_MAX];
    if (update || maxticks != max) {
	maxticks = max;
	rsrcd->set_rlimits(owner,
			   ({
			      resources["stack"][RSRC_MAX],
			      max,
			      rsrc[RSRC_DECAYTIME]
			   }));
    }
}

/*
 * NAME:	rsrc_set_limit()
 * DESCRIPTION:	set individual resource limit
 */
void rsrc_set_limit(string name, int max, int decay)
{
    if (previous_object() == rsrcd) {
	mixed *rsrc;

	if ((rsrc=resources[name])) {
	    rsrc[RSRC_MAX] = max;
	    if (name == "stack" || name == "ticks") {
		rlimits (-1; -1) {
		    set_rlimits(resources["ticks"], TRUE);
		}
	    }
	} else {
	    resources[name] = ({ (decay == 0) ? 0 : 0.0, max, 0 });
	}
    }
}

/*
 * NAME:	rsrc_set_maxtickusage()
 * DESCRIPTION:	set the maximum tick usage
 */
void rsrc_set_maxtickusage(float tickusage)
{
    if (previous_object() == rsrcd) {
	rlimits (-1; -1) {
	    maxusage = tickusage;
	    set_rlimits(resources["ticks"], FALSE);
	}
    }
}

/*
 * NAME:	rsrc_get()
 * DESCRIPTION:	get individual resource usage
 */
mixed *rsrc_get(string name, int *grsrc)
{
    if (previous_object() == rsrcd) {
	mixed *rsrc;
	int time;

	rsrc = resources[name];
	if (!rsrc) {
	    return ({ (grsrc[GRSRC_DECAY] == 0) ? 0 : 0.0,
		      grsrc[GRSRC_MAX], 0 }) +
		   grsrc[GRSRC_DECAY .. GRSRC_PERIOD];
	} else {
	    if ((int) grsrc[GRSRC_DECAY] != 0 &&
		(time=status(ST_UPTIME)) - (int) rsrc[RSRC_DECAYTIME] >=
							grsrc[GRSRC_PERIOD]) {
		rlimits (-1; -1) {
		    /* decay resource */
		    decay_rsrc(rsrc, grsrc, time);
		    if (name == "ticks") {
			set_rlimits(rsrc, TRUE);
		    }
		}
	    }
	    rsrc += grsrc[GRSRC_DECAY .. GRSRC_PERIOD];
	    if ((int) rsrc[RSRC_MAX] < 0) {
		rsrc[RSRC_MAX] = grsrc[GRSRC_MAX];
	    }
	    return rsrc;
	}
    }
}

/*
 * NAME:	rsrc_get_maxtickusage()
 * DESCRIPTION:	get maximum tick usage
 */
float rsrc_get_maxtickusage()
{
    if (previous_object() == rsrcd) {
	return maxusage;
    }
}

/*
 * NAME:	rsrc_incr()
 * DESCRIPTION:	increment or decrement a resource
 */
void rsrc_incr(string name, int incr, int *grsrc)
{
    if (previous_program() == RSRCD && incr != 0) {
	mapping tls, map, pending;
	mixed *arr;

	tls = TLS();
	map = TLSVAR(tls, TLS_RESOURCE);
	if (!map) {
	    map = TLSVAR(tls, TLS_RESOURCE) = ([ ]);
	}
	pending = map[this_object()];
	if (!pending) {
	    pending = map[this_object()] =
		      ([ name : ({ (float) incr, grsrc }) ]);
	    call_out("delayed_incr", 0, pending);
	} else {
	    arr = pending[name];
	    if (!arr) {
		pending[name] = ({ (float) incr, grsrc });
	    } else {
		arr[0] += (float) incr;
	    }
	}
    }
}

/*
 * NAME:	delayed_incr()
 * DESCRIPTION:	delayed increment or decrement of a resource
 */
static void delayed_incr(mapping map)
{
    string *names, name;
    mixed *values, *grsrc, *rsrc;
    int i, sz, time;
    float incr;

    names = map_indices(map);
    values = map_values(map);
    for (i = 0, sz = sizeof(names); i < sz; i++) {
	name = names[i];
	({ incr, grsrc }) = values[i];
	rsrc = resources[name];

	if (grsrc[GRSRC_DECAY] != 0) {
	    time = status(ST_UPTIME);
	    if (!rsrc) {
		/* new resource */
		rsrc = resources[name] = ({ 0.0, -1, time });
	    } else if (time - (int) rsrc[RSRC_DECAYTIME] >= grsrc[GRSRC_PERIOD])
	    {
		/* decay resource */
		decay_rsrc(rsrc, grsrc, time);
		time = 0;
	    }
	    rsrc[RSRC_USAGE] += incr;

	    if (name == "ticks") {
		set_rlimits(rsrc, time == 0);
	    }
	} else {
	    if (!rsrc) {
		/* new resource */
		rsrc = resources[name] = ({ 0, -1, 0 });
	    }
	    rsrc[RSRC_USAGE] += (int) incr;
	}
    }
}

/*
 * NAME:	decay_ticks()
 * DESCRIPTION:	decay ticks
 */
void decay_ticks(int *limits, int time, mixed *grsrc)
{
    if (previous_object() == rsrcd) {
	mixed *rsrc;

	rlimits (-1; -1) {
	    rsrc = resources["ticks"];
	    decay_rsrc(rsrc, grsrc, time);
	    maxticks = (maxusage > 0.0 && rsrc[RSRC_USAGE] >= maxusage) ?
			1 : rsrc[RSRC_MAX];
	    limits[LIM_MAX_TICKS] = maxticks;
	    limits[LIM_MAX_TIME] = rsrc[RSRC_DECAYTIME];
	}
    }
}

/*
 * NAME:	update_ticks()
 * DESCRIPTION:	update ticks for the current owner
 */
void update_ticks(int ticks, mixed *grsrc)
{
    if (previous_program() == RSRCD) {
	call_out_summand("incr_ticks", 0, (float) ticks, grsrc);
    }
}

/*
 * NAME:	incr_ticks()
 * DESCRIPTION:	increase ticks
 */
static void incr_ticks(float ticks, int *grsrc)
{
    mixed *rsrc;
    int time;

    rsrc = resources["ticks"];
    time = status(ST_UPTIME);
    if (time - (int) rsrc[RSRC_DECAYTIME] >= grsrc[GRSRC_PERIOD]) {
	/* decay resource */
	decay_rsrc(rsrc, grsrc, time);
	time = 0;
    }

    rsrc[RSRC_USAGE] += ticks;
    set_rlimits(rsrc, time == 0);
}
