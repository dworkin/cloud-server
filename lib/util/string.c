/*
 * NAME:        spaces()
 * DESCRIPTION: returns the number of spaces requested
 */
static string spaces(int width)
{
    string str;

    /* 63 spaces */
    str = "                                                               ";
    while (strlen(str) < width) {
        str += str;
    }
    return str[.. width - 1];
}

/*
 * NAME:	pad_left()
 * DESCRIPTION:	pad with spaces to the left
 */
static string pad_left(mixed str, int width)
{
    str = (string) str;
    if (strlen(str) >= width) {
	return str;
    }
    str = spaces(width) + str;
    return str[strlen(str) - width ..];
}

/*
 * NAME:	pad_right()
 * DESCRIPTION:	pad with spaces to the right
 */
static string pad_right(mixed str, int width)
{
    str = (string) str;
    if (strlen(str) >= width) {
	return str;
    }
    return (str + spaces(width))[.. width - 1];
}

/*
 * NAME:        break_line()
 * DESCRIPTION: break up a string into separate lines
 */
private string *break_line(string str, int width)
{
    int i, sz, lines, linelen, len, first;
    string *words;

    words = explode(str, " ");

    for (i = lines = 0, sz = sizeof(words); i < sz; lines++) {
	/* skip leading spaces */
	while ((linelen=strlen(words[i])) == 0) {
	    if (++i == sz) {
		/* skip this line, which contains only spaces */
		return words[.. lines - 1];
	    }
	}
	first = i;

	/* fill up line with words */
	while (++i < sz && linelen + (len = strlen(words[i])) < width) {
	    linelen += 1 + len;
	}
	words[lines] = implode(words[first .. i - 1], " ");
    }

    return words[.. lines - 1];
}

/*
 * NAME:        break_string()
 * DESCRIPTION: distribute words in a string over several lines
 */
static string break_string(string str, varargs int width, int indent)
{
    int i;
    string prefix, separator, *lines;

    if (width == 0) {
        width = 78;
    }
    width -= indent;
    prefix = spaces(indent);
    separator = "\n" + prefix;
    lines = explode("\n" + str, "\n");
    for (i = sizeof(lines); --i >= 0; ) {
        lines[i] = prefix + implode(break_line(lines[i], width), separator);
    }
    return implode(lines, "\n") + "\n";
}

/*
 * NAME:        indent_string()
 * DESCRIPTION: given a prefix, indent a string enough spaces to the right
 */
static string indent_string(string prefix, string str, varargs int width)
{
    int len;

    len = strlen(prefix);
    return prefix + break_string(str, width, len)[len ..];
}


/*
 * NAME:        capitalize()
 * DESCRIPTION: capitalize a string (ISO 8859-2)
 */
static string capitalize(string str)
{
    if (strlen(str) != 0) {
	int c;

	c = str[0];
	if ((c >= 'a' && c <= 'z') || (c >= 0xe0 && c <= 0xfe && c != 0xf7)) {
	    str[0] -= 'a' - 'A';
	}
    }
    return str;
}

/*
 * NAME:        lower_case()
 * DESCRIPTION: convert a string to lower case (ISO 8859-2)
 */
static string lower_case(string str)
{
    int i, c;

    for (i = strlen(str); --i >= 0; ) {
	c = str[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 0xc0 && c <= 0xde && c != 0xd7)) {
	    str[i] += 'a' - 'A';
	}
    }
    return str;
}

/*
 * NAME:        upper_case()
 * DESCRIPTION: convert a string to upper case (ISO 8859-2)
 */
static string upper_case(string str)
{
    int i, c;

    for (i = strlen(str); --i >= 0; ) {
	c = str[i];
        if ((c >= 'a' && c <= 'z') || (c >= 0xe0 && c <= 0xfe && c != 0xf7)) {
	    str[i] -= 'a' - 'A';
	}
    }
    return str;
}

/*
 * NAME:	tabulate()
 * DESCRIPTION:	show elements of arrays in different columns
 */
static string tabulate(mixed *columns...)
{
    int i, j, sz, lines, len, *indent;
    mixed *column;
    string str, *result;

    sz = sizeof(columns);
    indent = allocate_int(sz);

    /* compute number of lines and width of each column */
    for (i = 0; i < sz - 1; i++) {
	column = columns[i];
	if (lines < sizeof(column)) {
	    lines = sizeof(column);
	}
	for (j = sizeof(column); --j >= 0; ) {
	    if (column[j] != nil) {
		len = strlen((string) column[j]);
		if (indent[i + 1] < len) {
		    indent[i + 1] = len;
		}
	    }
	}
    }
    if (lines < sizeof(columns[i])) {
	lines = sizeof(columns[i]);
    }

    /* compute indent of each column */
    for (i = 1; i < sz; i++) {
	if (indent[i] != 0) {
	    indent[i]++;
	}
	indent[i] += indent[i - 1];
    }

    /* tabulate arrays in string */
    result = allocate(lines);
    for (j = 0; j < lines; j++) {
	str = "";
	for (i = 0; i < sz; i++) {
	    column = columns[i];
	    if (j < sizeof(column) && column[j] != nil) {
		str = pad_right(str, indent[i]);
		str += column[j];
	    }
	}
	result[j] = str;
    }
    return implode(result, "\n") + "\n";
}
