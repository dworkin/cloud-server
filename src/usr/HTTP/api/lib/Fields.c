# include "HttpField.h"
# include <Iterator.h>
# include <type.h>

inherit HttpFieldItem;
inherit Iterable;

private inherit "/lib/util/ascii";


private HttpField *fields;	/* fields in listed order */
private mapping map;		/* fields mapping */

/*
 * initialize fields object
 */
static void create()
{
    fields = ({ });
    map = ([ ]);
}

/*
 * add a new unique field
 */
static void addField(string name, mixed value, varargs mixed *params)
{
    string lcName;
    HttpField field;

    lcName = lower_case(name);
    if (map[lcName]) {
	error("Field already exists");
    }
    field = new HttpField(name, value, params);
    fields += ({ field });
    map[lcName] = field;
}

/*
 * add a new list-field
 */
static void addFieldList(string name, mixed *value, varargs mixed *params)
{
    string lcName;
    HttpField field;

    lcName = lower_case(name);
    field = map[lcName];
    if (field) {
	field->add(value, params);
    } else {
	field = new HttpField(name, value, params);
	fields += ({ field });
	map[lcName] = field;
    }
}

/*
 * add a field
 */
void add(HttpField field)
{
    string name;
    mixed value, *params;
    HttpField field2;

    name = field->lcName();
    value = field->value();
    params = field->params();
    field2 = map[name];
    if (field2) {
	if (typeof(value) == T_ARRAY) {
	    field2->add(value, params);
	} else {
	    error("Field already exists");
	}
    } else {
	fields += ({ field });
	map[name] = field;
    }
}

/*
 * remove a field
 */
void del(HttpField field)
{
    map[field->lcName()] = nil;
    fields -= ({ field });
}

/*
 * get field by name
 */
HttpField get(string name)
{
    return map[lower_case(name)];
}

/*
 * transport fields
 */
string transport()
{
    int sz, i;
    string *results;

    sz = sizeof(fields);
    results = allocate(sz);
    for (i = 0; i < sz; i++) {
	results[i] = fields[i]->transport();
    }

    return implode(results, "\r\n") + "\r\n";
}


/*
 * reset field iterator
 */
mixed iteratorStart(mixed from, mixed to)
{
    return 0;
}

/*
 * field iterator next
 */
mixed *iteratorNext(mixed state)
{
    int index;

    index = (int) state;
    return (index < sizeof(fields)) ?
	    ({ index + 1, fields[index] }) : ({ index, nil });
}

/*
 * field iterator end?
 */
int iteratorEnd(mixed state)
{
    return ((int) state >= sizeof(fields));
}
