/*
 * return empty array
 */
static mixed *empty(mixed *parsed)
{
    return ({ });
}

/*
 * return array of parsed elements
 */
static mixed *list(mixed *parsed)
{
    return ({ parsed });
}

/*
 * TRUE
 */
static mixed *true(mixed *parsed)
{
    return ({ TRUE });
}

/*
 * FALSE
 */
static mixed *false(mixed *parsed)
{
    return ({ FALSE });
}

/*
 * missing optional element
 */
static mixed *opt(mixed *parsed)
{
    return ({ nil });
}

/*
 * number of elements
 */
static mixed *count(mixed *parsed)
{
    return ({ sizeof(parsed) });
}

/*
 * concatenate elements
 */
static mixed *concat(mixed *parsed)
{
    return ({ implode(parsed, "") });
}

/*
 * element[1]
 */
static mixed *parsed_1_(mixed *parsed)
{
    return ({ parsed[1] });
}

/*
 * element[0], element[2]
 */
static mixed *parsed_0_2_(mixed *parsed)
{
    return ({ parsed[0], parsed[2] });
}
