# include <attributes.h>
# include <limits.h>


/*
 * health = CONbase * (10 + CON - CONbase) / 10
 * stamina = CON + 50
 * mana = WIL
 */

mixed **skill_info;	/* information about skills */
int **skill_list;	/* list of skills that depend on a stat */
int *exptab;		/* experience table */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize attribute server
 */
static void create()
{
    int i, j, *list, *ref;
    float f1, f2, exp;

    skill_info = ({
	({ "Sword",		STAT_STR, STAT_DEX }),
	({ "Shield",		STAT_DEX, STAT_QUI }),
	({ "Unarmed Combat",	STAT_STR, STAT_DEX }),
	({ "Bow",		STAT_STR, STAT_DEX }),
	({ "Evasion",		STAT_DEX, STAT_QUI }),
	({ "Healing",		STAT_DEX, STAT_FOC }),
	({ "Run",		STAT_QUI, STAT_QUI }),
	({ "Jump",		STAT_STR, STAT_DEX }),
	({ "Fletching",		STAT_DEX, STAT_FOC }),
	({ "Fire Magic",	STAT_FOC, STAT_WIL }),
	({ "Water Magic",	STAT_FOC, STAT_WIL }),
	({ "Earth Magic",	STAT_FOC, STAT_WIL }),
	({ "Air Magic",		STAT_FOC, STAT_WIL }),
	({ "Magic Resistance",	STAT_FOC, STAT_WIL })
    });

    skill_list = allocate(STAT_SIZE);
    for (i = 0; i < STAT_SIZE; i++) {
	list = ({ });
	for (j = 0; j < SKILL_SIZE; j++) {
	    ref = skill_info[j];
	    if (ref[1] == i || ref[2] == i) {
		list += ({ j });
	    }
	}
	skill_list[i] = list;
    }

    /*
     * The experience table defines how much experience is needed for each
     * level.  Note that this doesn't say much about how hard each level is;
     * instead, it determines how hard it is to reach the next level, if you
     * keep doing exactly the same tasks.
     *
     * Design criteria:
     *  - at the lowest levels, the amount of experience almost doubles every
     *    level
     *  - at the highest levels, the amount of experience doubles about every
     *    10 levels
     */
    exptab = allocate_int(MAX_LEVEL + 1);
    f1 = pow(2.0, .1);		/* pow(f1, 10) == 2 */
    f2 = 2.0 - f1;		/* f1 + f2 == 2, f2 < 1 */
    /* f1 + pow(f2, 1) == 2, f1 + pow(f2, i) approaches f1 for large i */
    for (i = MAX_LEVEL, exp = (float) INT_MAX + 42.0; i >= 0; --i) {
	exptab[i] = (int) (exp - 42.0);
	exp /= f1 + pow(f2, (float) (i + 1));
    }
}

/*
 * NAME:	adjust_stat()
 * DESCRIPTION:	a stat has changed; adjust skills and attrs as well
 */
void adjust_stat(int stat, int *stats, int *base_stats, int *skills,
		 int *incr_skills, int *buff_skills, int *attrs, int *max_attrs)
{
    int i, sz, skill, val, *list, *ref;

    list = skill_list[stat];
    for (i = 0, sz = sizeof(list); i < sz; i++) {
	skill = list[i];
	ref = skill_info[skill];
	val = (stats[ref[1]] + stats[ref[2]]) / 4 +
	      incr_skills[skill] + buff_skills[skill];
	if (val < 0) {
	    val = 0;
	}
	skills[skill] = val;
    }
    switch (stat) {
    case STAT_CON:
	/* health */
	i = base_stats[STAT_CON];
	i = i * (10 + stats[STAT_CON] - i) / 10;
	if (i <= 0) {
	    i = 1;
	}
	if (max_attrs[ATTR_HP] != 0) {
	    attrs[ATTR_HP] = attrs[ATTR_HP] * i / max_attrs[ATTR_HP];
	}
	max_attrs[ATTR_HP] = i;
	/* stamina */
	i = stats[STAT_CON] + 50;
	if (max_attrs[ATTR_STAM] != 0) {
	    attrs[ATTR_STAM] = attrs[ATTR_STAM] * i / max_attrs[ATTR_STAM];
	}
	max_attrs[ATTR_STAM] = i;
	break;

    case STAT_WIL:
	/* mana */
	i = stats[STAT_WIL];
	if (max_attrs[ATTR_MANA] != 0) {
	    attrs[ATTR_MANA] = attrs[ATTR_MANA] * i / max_attrs[ATTR_MANA];
	}
	max_attrs[ATTR_MANA] = i;
	break;
    }
}

/*
 * NAME:	adjust_skill()
 * DESCRIPTION:	adjust the value of a particular skill
 */
int adjust_skill(int skill, int incr, int buff, int *stats)
{
    int *ref, val;

    ref = skill_info[skill];
    val = (stats[ref[1]] + stats[ref[2]]) / 4 + incr + buff;
    return (val >= 0) ? val : 0;
}

/*
 * NAME:	query_level()
 * DESCRIPTION:	return the appropriate level for the given amount of experience
 */
int query_level(int level, int exp)
{
    for (;;) {
	level++;
	if (exptab[level] >= exp) { 
	    return level;
	}
    }
}

/*
 * NAME:	set_level()
 * DESCRIPTION:	adjust attributes for a specific level
 */
void set_level(int level, int *stats, int *base_stats, int *skills,
	       int *incr_skills, int *buff_skills, int *attrs, int *max_attrs)
{
    int i, val, *ref;

    /* skills */
    for (i = 0; i < SKILL_SIZE; i++) {
	ref = skill_info[i];
	val = (stats[ref[1]] + stats[ref[2]]) / 4 + incr_skills[i] +
	      buff_skills[i];
	if (val < 0) {
	    val = 0;
	}
	skills[i] = val;
    }

    /* exp */
    attrs[ATTR_EXP] = exptab[level - 1] + 1;
    max_attrs[ATTR_EXP] = exptab[level];
    /* health */
    i = base_stats[STAT_CON];
    i = i * (10 + stats[STAT_CON] - i) / 10;
    if (i <= 0) {
	i = 1;
    }
    if (max_attrs[ATTR_HP] != 0) {
	attrs[ATTR_HP] = attrs[ATTR_HP] * i / max_attrs[ATTR_HP];
    }
    max_attrs[ATTR_HP] = i;
    /* stamina */
    i = stats[STAT_CON] + 50;
    if (max_attrs[ATTR_STAM] != 0) {
	attrs[ATTR_STAM] = attrs[ATTR_STAM] * i / max_attrs[ATTR_STAM];
    }
    max_attrs[ATTR_STAM] = i;
    /* mana */
    i = stats[STAT_WIL];
    if (max_attrs[ATTR_MANA] != 0) {
	attrs[ATTR_MANA] = attrs[ATTR_MANA] * i / max_attrs[ATTR_MANA];
    }
    max_attrs[ATTR_MANA] = i;
}

/*
 * NAME:	query_exp_tasks()
 * DESCRIPTION:	return the number of equal-level tasks to perform at a given
 *		level
 */
int query_exp_tasks(int level)
{
    /* all right, this is completely arbitrary and wrong */
    return (int) (35.0 * pow(1.06, (float) level));
}

/*
 * NAME:	query_spec_points()
 * DESCRIPTION:	return the amount of specialization points available at a
 *		given level
 */
int query_spec_points(int level)
{
    int specpoints;

    specpoints = 0;
    if (level > 99) {
	level = 99;
    }
    if (level > 39) {
	specpoints = level - 39;
	level = 39;
    }
    if (level > 12) {
	specpoints += 2 * (level - 12);
	level = 12;
    }
    return specpoints + 3 * level;
}

/*
 * NAME:	query_skill_incr()
 * DESCRIPTION:	return the proper skill increment at a given level
 */
int query_skill_incr(int specpoints, int level)
{
    float incr;

    /*
     * step 1: calculate skill increment for given spec points 0..50 -> 0..150
     * Increment initially rises quickly, later more slowly.
     */
    incr = (float) specpoints;
    incr *= 6.0 - incr * .06;
    /*
     * step 2:
     * Adjust for levels below 100.
     */
    if (level < 100) {
	incr *= log10(5.0 + (float) level / 20.0);	/* ~.7 .. 1.0 */
    }
    return (int) floor(incr);
}
