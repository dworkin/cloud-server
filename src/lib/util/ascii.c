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

/*
 * float to string, with full accuracy
 */
static string float2string(float flt)
{
    string str, str2, str3, exp;
    float flt2;

    str = (string) flt;
    flt2 = (float) str;
    sscanf(str, "%se%s", str, exp);

    if (flt != flt2) {
	str2 = str;
	str2[strlen(str2) - 1]--;
	str3 = (string) fabs(flt - (float) (str2 + exp));
	if (fabs(flt) > fabs(flt2)) {
	    /* string representation was rounded down */
	    str2 = str3[1 ..];
	    sscanf(str2, ".%s", str2);
	    sscanf(str2, "%se", str2);
	} else {
	    /* string representation was rounded up */
	    str = str2;
	    str2 = str3;
	    sscanf(str2, "%se", str2);
	    if (sscanf(str2, "%s.%s", str2, str3) != 0) {
		str2 += str3;
	    }
	}

	/* extend mantissa */
	if (sscanf(str, "%*s.") == 0) {
	    str += ".";
	}
	str += str2;
    }

    if (exp) {
	str += "e" + exp;
    } else if (sscanf(str, "%*s.") == 0) {
	str += ".0";
    }

    return str;
}
