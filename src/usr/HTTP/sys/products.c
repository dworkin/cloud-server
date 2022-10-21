# include "HttpField.h"

/*
 * Server and User-Agent
 */
mixed *products(string str)
{
    return parse_string("\
ws = /[ \t]+/								\
token = /[-!#$%&'*+.^_`|~A-Za-z0-9]+/					\
comment = /\\(([^\x00-\x08\x0a-\x1f\x7f()\\\\]|\\(([^\x00-\x08\x0a-\x1f\x7f()\\\\]|\\(([^\x00-\x08\x0a-\x1f\x7f()\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\\)|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\\)|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\\)/	\
junk = /./								\
\
Products: Product							\
Products: Products Ws Product						\
\
Ws: ws							? null		\
\
Product: token Comments					? product1	\
Product: token '/' token Comments			? product2	\
\
Comments:								\
Comments: Comments Ws comment", str);
}

/*
 * nothing
 */
static mixed null(mixed parsed)
{
    return ({ });
}

/*
 * product comments
 */
static mixed *product1(mixed *parsed)
{
    return ({ new HttpProduct(parsed[0], nil, parsed[1 ..]...) });
}

/*
 * product/version comments
 */
static mixed *product2(mixed *parsed)
{
    return ({ new HttpProduct(parsed[0], parsed[2 ..]...) });
}
