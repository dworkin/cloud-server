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

/*
 * escape ", \n and \ in a string
 */
static string stringify(string str)
{
    if (sscanf(str, "%*s\\") != 0) {
	str = implode(explode("\\" + str + "\\", "\\"), "\\\\");
    }
    if (sscanf(str, "%*s\"") != 0) {
	str = implode(explode("\"" + str + "\"", "\""), "\\\"");
    }
    if (sscanf(str, "%*s\n") != 0) {
	str = implode(explode("\n" + str + "\n", "\n"), "\\n");
    }

    return str;
}
