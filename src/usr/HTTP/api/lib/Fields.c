# include <Time.h>
# include <type.h>
# include "HttpField.h"

inherit HttpFieldItem;

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
static void addField(string name, mixed value)
{
    string lcName;
    HttpField field;

    lcName = lower_case(name);
    if (map[lcName]) {
	error("Field already exists");
    }
    field = new HttpField(name, value);
    fields += ({ field });
    map[lcName] = field;
}

/*
 * add a new list-field
 */
static void addFieldList(string name, mixed *value)
{
    string lcName;
    HttpField field;

    lcName = lower_case(name);
    field = map[lcName];
    if (field) {
	field->add(value);
    } else {
	field = new HttpField(name, value);
	fields += ({ field });
	map[lcName] = field;
    }
}

/*
 * add a field
 */
void add(HttpField field)
{
    string name, lcName;
    mixed value;

    name = field->name();
    value = field->value();
    lcName = lower_case(name);
    if (map[lcName]) {
	if (typeof(value) == T_ARRAY) {
	    addFieldList(name, value);
	} else {
	    error("Field already exists");
	}
    } else {
	fields += ({ field });
	map[lcName] = field;
    }
}

/*
 * remove a field
 */
void del(HttpField field)
{
    map[lower_case(field->name())] = nil;
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

    return implode(results, "\r\n") + "\r\n\r\n";
}


HttpField *fields()	{ return fields[..]; }
