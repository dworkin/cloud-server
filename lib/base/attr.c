# include <attributes.h>


/*
 * level-related attributes
 */

private int *stats;		/* effective stats */
private int *base_stats;	/* base stats */
private int *buff_stats;	/* stat (de)buffs */
private int *skills;		/* effective skills */
private int *incr_skills;	/* skill increments */
private int *buff_skills;	/* skill (de)buffs */
private int exp;		/* current experience */
private int hitpoints;		/* current health */
private int stamina;		/* current stamina */
private int mana;		/* current magical power */
private int max_exp;		/* > this and you advance in level */
private int max_hitpoints;	/* maximum health */
private int max_stamina;	/* maximum stamina */
private int max_mana;		/* maximum mana */
private int level;		/* 1..100 for normal players */
private object attrd;		/* attribute server */


/*
 * NAME:	create()
 * DESCRIPTION:	initialize living thing
 */
static void create()
{
    if (!stats) {
	stats =       allocate_int(STAT_SIZE);
	base_stats =  allocate_int(STAT_SIZE);
	buff_stats =  allocate_int(STAT_SIZE);
	skills =      allocate_int(SKILL_SIZE);
	incr_skills = allocate_int(SKILL_SIZE);
	buff_skills = allocate_int(SKILL_SIZE);
	attrd = find_object(ATTRD);
    }
}

/*
 * NAME:	set_base_stat()
 * DESCRIPTION:	Set a base stat; for a player, in range 10-100.
 *		Not supposed to change later on.
 */
void set_base_stat(int stat, int base)
{
    if (base_stats[stat] != base) {
	if (base < 0) {
	    error("Invalid base stat value");
	}
	base_stats[stat] = base;
	if (level != 0) {
	    int val, *attrs, *max_attrs;

	    val = base + level + buff_stats[stat];
	    if (val <= 0) {
		val = 1;
	    }
	    stats[stat] = val;
	    attrs =     ({ 0, hitpoints,     stamina,     mana });
	    max_attrs = ({ 0, max_hitpoints, max_stamina, max_mana });
	    attrd->adjust_stat(stat, stats, base_stats, skills, incr_skills,
			       buff_skills, attrs, max_attrs);
	    hitpoints =     attrs[ATTR_HP];
	    stamina =       attrs[ATTR_STAM];
	    mana =          attrs[ATTR_MANA];
	    max_hitpoints = max_attrs[ATTR_HP];
	    max_stamina =   max_attrs[ATTR_STAM];
	    max_mana =      max_attrs[ATTR_MANA];
	}
    }
}

/*
 * NAME:	buff_stat()
 * DESCRIPTION:	set the temporary (de)buff of a stat
 */
void buff_stat(int stat, int buff)
{
    if (buff_stats[stat] != buff) {
	int val, *attrs, *max_attrs;

	buff_stats[stat] = buff;
	val = base_stats[stat] + level + buff;
	if (val <= 0) {
	    val = 1;
	}
	stats[stat] = val;
	attrs =     ({ 0, hitpoints,     stamina,     mana });
	max_attrs = ({ 0, max_hitpoints, max_stamina, max_mana });
	attrd->adjust_stat(stat, stats, base_stats, skills, incr_skills,
			   buff_skills, attrs, max_attrs);
	hitpoints =     attrs[ATTR_HP];
	stamina =       attrs[ATTR_STAM];
	mana =          attrs[ATTR_MANA];
	max_hitpoints = max_attrs[ATTR_HP];
	max_stamina =   max_attrs[ATTR_STAM];
	max_mana =      max_attrs[ATTR_MANA];
    }
}

/*
 * NAME:	set_skill_incr()
 * DESCRIPTION:	Set the increment of a skill; this happens all the time as the
 *		player uses his skills.
 */
void set_skill_incr(int skill, int incr)
{
    if (incr != incr_skills[skill]) {
	if (incr < 0) {
	    error("Invalid skill increment");
	}
	incr_skills[skill] = incr;
	skills[skill] = attrd->adjust_skill(skill, incr, buff_skills[skill],
					    stats);
    }
}

/*
 * NAME:	buff_skill()
 * DESCRIPTION:	set the temporary (de)buff of a skill
 */
void buff_skill(int skill, int buff)
{
    if (buff != buff_skills[skill]) {
	buff_skills[skill] = buff;
	skills[skill] = attrd->adjust_skill(skill, incr_skills[skill], buff,
					    stats);
    }
}

/*
 * NAME:	add_exp()
 * DESCRIPTION:	gain experience
 */
int add_exp(int incr)
{
    incr += exp;
    if (incr > max_exp) {
	int i, val, *attrs, *max_attrs;

	/*
	 * gain a level, too
	 */
	level = attrd->query_level(level, exp = incr);
	for (i = 0; i < STAT_SIZE; i++) {
	    val = base_stats[i] + level + buff_stats[i];
	    if (val <= 0) {
		val = 1;
	    }
	    stats[i] = val;
	}
	attrs =     ({ exp, hitpoints,     stamina,     mana });
	max_attrs = ({ 0,   max_hitpoints, max_stamina, max_mana });
	attrd->set_level(level, stats, base_stats, skills, incr_skills,
			 buff_skills, attrs, max_attrs);
	hitpoints =     attrs[ATTR_HP];
	stamina =       attrs[ATTR_STAM];
	mana =          attrs[ATTR_MANA];
	max_exp =       max_attrs[ATTR_EXP];
	max_hitpoints = max_attrs[ATTR_HP];
	max_stamina =   max_attrs[ATTR_STAM];
	max_mana =      max_attrs[ATTR_MANA];
    } else {
	if (incr < 0) {
	    incr = 0;
	}
	if (exp != incr) {
	    attrd->adjust_exp(exp = incr);
	}
    }

    return exp;
}

/*
 * NAME:	add_hitpoints()
 * DESCRIPTION:	increase or decrease hitpoints
 */
int add_hitpoints(int val)
{
    val += hitpoints;
    if (val < 0) {
	val = 0;
    } else if (val > max_hitpoints) {
	val = max_hitpoints;
    }
    if (val != hitpoints) {
	attrd->adjust_hitpoints(hitpoints = val);
    }

    return hitpoints;
}

/*
 * NAME:	add_stamina()
 * DESCRIPTION:	increase or decrease stamina
 */
int add_stamina(int val)
{
    val += stamina;
    if (val < 0) {
	val = 0;
    } else if (val > max_stamina) {
	val = max_stamina;
    }
    if (val != stamina) {
	attrd->adjust_stamina(stamina = val);
    }

    return stamina;
}

/*
 * NAME:	add_mana()
 * DESCRIPTION:	increase or decrease mana
 */
int add_mana(int val)
{
    val += mana;
    if (val < 0) {
	val = 0;
    } else if (val > max_mana) {
	val = max_mana;
    }
    if (val != mana) {
	attrd->adjust_mana(mana = val);
    }

    return mana;
}

/*
 * NAME:	set_level()
 * DESCRIPTION:	change the current level
 */
void set_level(int lev)
{
    int i, val, *attrs, *max_attrs;

    if (lev <= 0 || lev > MAX_LEVEL) {
	error("Invalid level");
    }

    level = lev;
    for (i = 0; i < STAT_SIZE; i++) {
	val = base_stats[i] + lev + buff_stats[i];
	if (val <= 0) {
	    val = 1;
	}
	stats[i] = val;
    }
    attrs =     ({ 0, 0,             0,           0 });
    max_attrs = ({ 0, max_hitpoints, max_stamina, max_mana });
    attrd->set_level(level, stats, base_stats, skills, incr_skills, buff_skills,
		     attrs, max_attrs);
    exp =       attrs[ATTR_EXP];
    max_exp =   max_attrs[ATTR_EXP];
    hitpoints = max_hitpoints = max_attrs[ATTR_HP];
    stamina =   max_stamina =   max_attrs[ATTR_STAM];
    mana =      max_mana =      max_attrs[ATTR_MANA];
}

/*
 * attribute query functions
 */
int query_stat(int stat)	{ return stats[stat]; }
int query_base_stat(int stat)	{ return base_stats[stat]; }
int query_stat_buff(int stat)	{ return buff_stats[stat]; }
int *query_stats()		{ return stats[..]; }
int query_skill(int skill)	{ return skills[skill]; }
int query_skill_incr(int skill)	{ return incr_skills[skill]; }
int query_skill_buff(int skill)	{ return buff_skills[skill]; }
int *query_skills()		{ return skills[..]; }
int query_exp()			{ return exp; }
int query_hitpoints()		{ return hitpoints; }
int query_stamina()		{ return stamina; }
int query_mana()		{ return mana; }
int *query_attrs()		{ return ({ exp, hitpoints, stamina, mana }); }
int query_max_exp()		{ return max_exp; }
int query_max_hitpoints()	{ return max_hitpoints; }
int query_max_stamina()		{ return max_stamina; }
int query_max_mana()		{ return max_mana; }
int query_level()		{ return level; }
