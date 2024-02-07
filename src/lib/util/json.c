# define JSONENCODE	"/sys/jsonencode"
# define JSONDECODE	"/sys/jsondecode"

/*
 * encode an LPC value as JSON
 */
static string encode(mixed value)
{
    return JSONENCODE->encode(value);
}

/*
 * decode a JSON string to LPC
 */
static mixed decode(string str)
{
    return JSONDECODE->decode(str);
}
