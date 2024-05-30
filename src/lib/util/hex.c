# include <limits.h>

/*
 * encode a number in hexadecimal (lower case)
 */
static string encode(int number, varargs int digits)
{
    string hex, str;
    int i;

    hex = "0123456789abcdef";
    str = "";
    do {
	i = number & 0xf;
	str = hex[i .. i] + str;
	number >>= 4;
    } while (--digits > 0 || number != 0);

    return str;
}

/*
 * encode a number in hexadecimal (upper case)
 */
static string encodeUpper(int number, varargs int digits)
{
    string hex, str;
    int i;

    hex = "0123456789ABCDEF";
    str = "";
    do {
	i = number & 0xf;
	str = hex[i .. i] + str;
	number >>= 4;
    } while (--digits > 0 || number != 0);

    return str;
}

/*
 * format a byte string as hexadecimal
 */
static string format(string bytes)
{
    string hex, str, *result;
    int i, x;

    hex = "0123456789abcdef";
    str = "..";
    result = allocate(i=strlen(bytes));
    while (--i >= 0) {
	x = bytes[i];
	str[0] = hex[x >> 4];
	str[1] = hex[x & 0xf];
	result[i] = str;
    }

    return implode(result, "");
}

/*
 * format a byte string as upper hexadecimal
 */
static string formatUpper(string bytes)
{
    string hex, str, *result;
    int i, x;

    hex = "0123456789ABCDEF";
    str = "..";
    result = allocate(i=strlen(bytes));
    while (--i >= 0) {
	x = bytes[i];
	str[0] = hex[x >> 4];
	str[1] = hex[x & 0xf];
	result[i] = str;
    }

    return implode(result, "");
}

/*
 * decode a hexadecimal number
 */
static int decode(string str)
{
    int i, len, num, c;

    len = strlen(str) - 1;

    for (i = 0; str[i] == '0' && i < len; i++) ;
# if INT_MAX == 0x7fffffff
    if (len - i >= 8) {
# else
    if (len - i >= 16) {
# endif
	error("Hexadecimal too large");
    }

    num = 0;
    for (; i <= len; i++) {
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

/*
 * decode a hexadecimal string
 */
static string decodeString(string str)
{
    int len, i, num, c;
    string byte, *bytes;

    len = strlen(str);
    if (len & 1) {
	str = "0" + str;
	len++;
    }

    byte = ".";
    bytes = allocate(len >> 1);
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
	num = c << 4;

	switch (c = str[++i]) {
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
	byte[0] = num | c;
	bytes[i >> 1] = byte;
    }

    return implode(bytes, "");
}
