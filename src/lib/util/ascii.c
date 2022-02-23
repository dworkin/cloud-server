/*
 * capitalize a string
 */
static string capitalize(string str)
{
    int c;

    if (strlen(str) != 0) {
	c = str[0];
	if (c >= 'a' && c <= 'z') {
	    str[0] = c - 'a' + 'A';
	}
    }
    return str;
}

/*
 * convert a string to lower case
 */
static string lower_case(string str)
{
    int i, c;

    for (i = strlen(str); --i >= 0; ) {
	c = str[i];
	if (c >= 'A' && c <= 'Z') {
	    str[i] = c + 'a' - 'A';
	}
    }
    return str;
}

/*
 * convert a string to upper case
 */
static string upper_case(string str)
{
    int i, c;

    for (i = strlen(str); --i >= 0; ) {
	c = str[i];
	if (c >= 'a' && c <= 'z') {
	    str[i] = c - 'a' + 'A';
	}
    }
    return str;
}
