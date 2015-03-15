/*
 * NAME:	decode()
 * DESCRIPTION:	decode UTF8 string
 */
mixed *decode(string str)
{
    catch {
	return parse_string("\
ascii = /[\x00-\x7f]+/							\
upper = /[\xc2-\xc3][\x80-\xbf]/					\
char2 = /[\xc4-\xdf][\x80-\xbf]/					\
char3 = /[\xe0-\xef][\x80-\xbf][\x80-\xbf]/				\
char4 = /[\xf0-\xf4][\x80-\xbf][\x80-\xbf][\x80-\xbf]/			\
partial = /[\xc2-\xdf]/							\
partial = /[\xe0-\xef]/							\
partial = /[\xe0-\xef][\x80-\xbf]/					\
partial = /[\xf0-\xf4]/							\
partial = /[\xf0-\xf4][\x80-\xbf]/					\
partial = /[\xf0-\xf4][\x80-\xbf][\x80-\xbf]/				\
\
UTF8: String				? full				\
UTF8: String Low			? full				\
UTF8: String partial			? partial			\
UTF8: String Low partial		? partial			\
\
String:									\
String: High								\
String: String Low High							\
\
Low: LowChar								\
Low: LowSeq LowChar			? merge				\
\
LowSeq: LowChar								\
LowSeq: LowSeq LowChar							\
\
LowChar: ascii								\
LowChar: upper				? upper				\
\
High: HighChar								\
High: HighSeq HighChar			? collect			\
\
HighSeq: HighChar							\
HighSeq: HighSeq HighChar						\
\
HighChar: char2				? char2				\
HighChar: char3				? char3				\
HighChar: char4				? char4", str);
    }
}

/*
 * NAME:	full()
 * DESCRIPTION:	UTF8 string without remainders
 */
static mixed *full(mixed *parsed)
{
    return ({ parsed, nil });
}

/*
 * NAME:	partial()
 * DESCRIPTION:	UTF8 string with remaining partial sequence
 */
static mixed *partial(mixed *parsed)
{
    int size;

    size = sizeof(parsed);
    return ({ parsed[.. size - 2], parsed[size - 1] });
}

/*
 * NAME:	collect()
 * DESCRIPTION:	collect high sequence in sub-array
 */
static mixed *collect(mixed *parsed)
{
    return ({ parsed });
}

/*
 * NAME:	merge()
 * DESCRIPTION:	merge low sequence in binary string
 */
static mixed *merge(mixed *parsed)
{
    return ({ implode(parsed, "") });
}

/*
 * NAME:	upper()
 * DESCRIPTION:	character in range 0x80-0xff
 */
static mixed *upper(mixed *parsed)
{
    string str;

    str = " ";
    str[0] = (parsed[0][0] << 6) + (parsed[0][1] & 0x3f);
    return ({ str });
}

/*
 * NAME:	char2()
 * DESCRIPTION:	character in range 0x100 - 0x7ff
 */
static mixed *char2(mixed *parsed)
{
    return ({ ((parsed[0][0] & 0x1f) << 6) + (parsed[0][1] & 0x3f) });
}

/*
 * NAME:	char3()
 * DESCRIPTION:	character in range 0x800 - 0xffff
 */
static mixed *char3(mixed *parsed)
{
    string str;
    int i;

    str = parsed[0];
    i = ((str[0] & 0xf) << 12) + ((str[1] & 0x3f) << 6) + (str[2] & 0x3f);
    return (i >= 0x800) ? ({ i }) : nil;
}

/*
 * NAME:	char4()
 * DESCRIPTION:	character in range 0x10000 - 0x10ffff
 */
static mixed *char4(mixed *parsed)
{
    string str;
    int i;

    str = parsed[0];
    i = ((str[0] & 0x7) << 18) + ((str[1] & 0x3f) << 12) +
	((str[2] & 0x3f) << 6) + (str[3] & 0x3f);
    return (i >= 0x10000 && i <= 0x10ffff) ? ({ i }) : nil;
}
