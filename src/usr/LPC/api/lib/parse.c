/*
 * NAME:	empty()
 * DESCRIPTION:	return empty array
 */
static mixed *empty(mixed *parsed)
{
    return ({ });
}

/*
 * NAME:	list()
 * DESCRIPTION:	return array of parsed elements
 */
static mixed *list(mixed *parsed)
{
    return ({ parsed });
}

/*
 * NAME:	true()
 * DESCRIPTION:	TRUE
 */
static mixed *true(mixed *parsed)
{
    return ({ TRUE });
}

/*
 * NAME:	fals()
 * DESCRIPTION:	FALSE
 */
static mixed *false(mixed *parsed)
{
    return ({ FALSE });
}

/*
 * NAME:	opt()
 * DESCRIPTION:	missing optional element
 */
static mixed *opt(mixed *parsed)
{
    return ({ nil });
}

/*
 * NAME:	count()
 * DESCRIPTION:	number of elements
 */
static mixed *count(mixed *parsed)
{
    return ({ sizeof(parsed) });
}

/*
 * NAME:	concat()
 * DESCRIPTION:	concatenate elements
 */
static mixed *concat(mixed *parsed)
{
    return ({ implode(parsed, "") });
}

/*
 * NAME:	parsed_1_()
 * DESCRIPTION:	element[1]
 */
static mixed *parsed_1_(mixed *parsed)
{
    return ({ parsed[1] });
}

/*
 * NAME:	parsed_0_2_()
 * DESCRIPTION:	element[0], element[2]
 */
static mixed *parsed_0_2_(mixed *parsed)
{
    return ({ parsed[0], parsed[2] });
}
