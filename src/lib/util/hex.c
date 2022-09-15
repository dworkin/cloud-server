# include <limits.h>

/*
 * encode a number in hexadecimal (lower case)
 */
static string encode(int number)
{
    string hex, str;
    int i;

    hex = "0123456789abcdef";
    str = "";
    do {
	i = number & 0xf;
	str = hex[i .. i] + str;
	number >>= 4;
    } while (number != 0);

    return str;
}

/*
 * encode a number in hexadecimal (upper case)
 */
static string encodeUpper(int number)
{
    string hex, str;
    int i;

    hex = "0123456789ABCDEF";
    str = "";
    do {
	i = number & 0xf;
	str = hex[i .. i] + str;
	number >>= 4;
    } while (number != 0);

    return str;
}

/*
 * decode a hexadecimal number
 */
static int decode(string str)
{
    int i, len, num, c;

    len = strlen(str) - 1;

    for (i = 0; str[i] == '0' && i < len; i++) ;
    str = str[i .. len];
    len -= i - 1;

# if INT_MAX == 0x7fffffff
    if (len > 8) {
# else
    if (len > 16) {
# endif
	error("Hexadecimal too large");
    }

    num = 0;
    for (i = 0; i < len; i++) {
	switch (c = str[i]) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    c -= '0';
	    break;

	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	    c -= 'a' - 10;
	    break;

	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	    c -= 'A' - 10;
	    break;

	default:
	    error("Bad digit in hexadecimal");
	}
	num = (num << 4) | c;
    }

    return num;
}
