# include <limits.h>

/*
 * encode number as ASN
 */
static string encode(int number)
{
    string str;

    switch (number) {
    case 0 .. 0x7f:
    case -1 ^ 0x7f .. -1:
	str = ".";
	str[0] = number;
	break;

    case 0x80 .. 0x7fff:
    case -1 ^ 0x7fff .. -1 ^ 0x80:
	str = "..";
	str[0] = number >> 8;
	str[1] = number;
	break;

    case 0x8000 .. 0x7fffff:
    case -1 ^ 0x7fffff .. -1 ^ 0x8000:
	str = "...";
	str[0] = number >> 16;
	str[1] = number >> 8;
	str[2] = number;
	break;

# if INT_MIN == 0x80000000
    default:
# else
    case 0x800000 .. 0x7fffffff:
    case -1 ^ 0x7fffffff .. -1 ^ 0x800000:
# endif
	str = "....";
	str[0] = number >> 24;
	str[1] = number >> 16;
	str[2] = number >> 8;
	str[3] = number;
	break;

# if INT_MIN != 0x80000000
    case 0x80000000 .. 0x7fffffffff:
    case -1 ^ 0x7fffffffff .. -1 ^ 0x80000000:
	str = ".....";
	str[0] = number >> 32;
	str[1] = number >> 24;
	str[2] = number >> 16;
	str[3] = number >> 8;
	str[4] = number;
	break;

    case 0x8000000000 .. 0x7fffffffffff:
    case -1 ^ 0x7fffffffffff .. -1 ^ 0x8000000000:
	str = "......";
	str[0] = number >> 40;
	str[1] = number >> 32;
	str[2] = number >> 24;
	str[3] = number >> 16;
	str[4] = number >> 8;
	str[5] = number;
	break;

    case 0x800000000000 .. 0x7fffffffffffff:
    case -1 ^ 0x7fffffffffffff .. -1 ^ 0x800000000000:
	str = ".......";
	str[0] = number >> 48;
	str[1] = number >> 40;
	str[2] = number >> 32;
	str[3] = number >> 24;
	str[4] = number >> 16;
	str[5] = number >> 8;
	str[6] = number;
	break;

    default:
	str = "........";
	str[0] = number >> 56;
	str[1] = number >> 48;
	str[2] = number >> 40;
	str[3] = number >> 32;
	str[4] = number >> 24;
	str[5] = number >> 16;
	str[6] = number >> 8;
	str[7] = number;
	break;
# endif
    }

    return str;
}

/*
 * decode ASN to number
 */
static int decode(string str)
{
    int number;

    number = str[0];
    if (number >= 0x80) {
	number |= -1 << 8;
    }

    switch (strlen(str)) {
    case 1:
	return number;

    case 2:
	return (number << 8) | str[1];

    case 3:
	return (number << 16) | (str[1] << 8) | str[2];

    case 4:
	return (number << 24) | (str[1] << 16) | (str[2] << 8) | str[3];

# if INT_MIN != 0x80000000
    case 5:
	return (number << 32) | (str[1] << 24) | (str[2] << 16) |
	       (str[3] << 8) | str[4];

    case 6:
	return (number << 40) | (str[1] << 32) | (str[2] << 24) |
	       (str[3] << 16) | (str[4] << 8) | str[5];

    case 7:
	return (number << 48) | (str[1] << 40) | (str[2] << 32) |
	       (str[3] << 24) | (str[4] << 16) | (str[5] << 8) | str[6];

    case 8:
	return (number << 56) | (str[1] << 48) | (str[2] << 40) |
	       (str[3] << 32) | (str[4] << 24) | (str[5] << 16) |
	       (str[6] << 8) | str[7];
# endif

    default:
	error("Number too big");
    }
}

/*
 * extend an ASN
 */
static string extend(string str, int length)
{
    string pad;

    if (strlen(str) < length) {
	pad = "\0\0\0\0\0\0\0\0";
	while (strlen(pad) + strlen(str) < length) {
	    pad += pad;
	}
	str = (pad + str)[strlen(pad) + strlen(str) - length ..];
    }

    return str;
}

/*
 * count bits in ASN
 */
static int bits(string str)
{
    int len, i, bits;

    for (len = strlen(str), i = 0; i < len; i++) {
	if (str[i] != '\0') {
	    switch (str[i]) {
	    case 0x01:		bits = 1; break;
	    case 0x02 .. 0x03:	bits = 2; break;
	    case 0x04 .. 0x07:	bits = 3; break;
	    case 0x08 .. 0x0f:	bits = 4; break;
	    case 0x10 .. 0x1f:	bits = 5; break;
	    case 0x20 .. 0x3f:	bits = 6; break;
	    case 0x40 .. 0x7f:	bits = 7; break;
	    case 0x80 .. 0xff:	bits = 8; break;
	    }

	    return (len - i - 1) * 8 + bits;
	}
    }

    return 0;
}
