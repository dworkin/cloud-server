/*
 * generate a pseudo-random string
 */
static string random_string(int length)
{
    int n, x;
    string *result, str;

    n = (length + 2) / 3;
    result = allocate(n);
    str = "...";
    for (str = "..."; --n >= 0; ) {
	x = random(0x1000000);
	str[0] = x;
	x >>= 8;
	str[1] = x;
	x >>= 8;
	str[2] = x;
	result[n] = str;
    }

    return implode(result, "")[.. length - 1];
}
