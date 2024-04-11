# include <Iterator.h>
# include <KVstore.h>
# include <type.h>

inherit Iterable;
private inherit "/lib/util/random";


/* ref = ({ index, key, value, changes }) */
# define INDEX		0
# define KEY		1
# define VALUE		2
# define CHANGES	3

private string accessKey;	/* KVstore access key */
private object root;		/* KVstore root object */

/*
 * create KVstore, including root object
 */
atomic static void create(int maxSize, varargs string accessKey,
			  string nodePath)
{
    ::accessKey = (accessKey) ? accessKey : random_string(32);
    root = clone_object((nodePath) ? nodePath : KVNODE, ::accessKey, maxSize);
}

/*
 * get the value associated with a key
 */
mixed get(string key)
{
    if (!key) {
	error("Invalid key");
    }
    return root->get(accessKey, key);
}

/*
 * set the value associated with a key, nil for deletion
 */
atomic void set(string key, mixed value)
{
    if (!key) {
	error("Invalid key");
    }
    if (typeof(value) == T_OBJECT && sscanf(object_name(value), "%*s#-1") == 0)
    {
	error("Invalid value");
    }
    root->set(accessKey, key, value, 0, nil, nil, nil, nil);
}

/*
 * set the value associated with a key, which must not yet exist
 */
atomic void add(string key, mixed value)
{
    if (!key) {
	error("Invalid key");
    }
    if (typeof(value) == T_OBJECT && sscanf(object_name(value), "%*s#-1") == 0)
    {
	error("Invalid value");
    }
    root->set(accessKey, key, value, -1, nil, nil, nil, nil);
}

/*
 * set the value associated with a key, which must already exist
 */
atomic void change(string key, mixed value)
{
    if (!key) {
	error("Invalid key");
    }
    if (typeof(value) == T_OBJECT && sscanf(object_name(value), "%*s#-1") == 0)
    {
	error("Invalid value");
    }
    root->set(accessKey, key, value, 1, nil, nil, nil, nil);
}

/*
 * remove a KVstore, deleting all nodes
 */
atomic void remove()
{
    root->remove(accessKey);
}

/*
 * KVstore[index] = value
 */
static void operator[]= (string index, mixed value)
{
    set(index, value);
}

/*
 * KVstore[index]
 */
static mixed operator[] (string index)
{
    return get(index);
}


/*
 * first item
 */
private mixed **stackFirst()
{
    mixed *ref, **stack;

    ref = root->refIndex(accessKey, -1, 0);
    if (ref[VALUE] == nil && !ref[KEY]) {
	return nil;		/* empty root */
    }

    for (stack = ({ ref }); !ref[KEY]; stack = ({ ref }) + stack) {
	ref = ref[VALUE]->refIndex(accessKey, -1, 0);
    }

    return stack;
}

/*
 * find item by key
 */
private mixed **stackKey(string key)
{
    mixed *ref, **stack;

    ref = root->refKey(accessKey, key);
    if (ref[VALUE] == nil && !ref[KEY]) {
	return nil;		/* empty root */
    }

    for (stack = ({ ref }); !ref[KEY]; stack = ({ ref }) + stack) {
	ref = ref[VALUE]->refKey(accessKey, key);
    }

    return stack;
}

/*
 * next item
 */
private mixed **stackNext(mixed **stack)
{
    string key;
    mixed *ref;
    object node;

    key = stack[0][KEY];

    do {
	ref = stack[0];
	stack = stack[1 ..];

	if (!(node=(sizeof(stack) != 0) ? stack[0][VALUE] : root) ||
	    !(ref=node->refIndex(accessKey, ref[CHANGES], ref[INDEX] + 1))) {
	    /*
	     * reference outdated: fall back to search by key
	     */
	    stack = stackKey(key);
	    if (stack && stack[0][KEY] == key) {
		continue;
	    }
	    return stack;
	}

	if (ref[VALUE] != nil || ref[KEY]) {
	    /*
	     * not out of range
	     */
	    for (;;) {
		stack = ({ ref }) + stack;
		if (ref[KEY]) {
		    /* leaf */
		    return stack;
		}
		ref = ref[VALUE]->refIndex(accessKey, -1, 0);
	    }
	}
    } while (sizeof(stack) != 0);
}

/*
 * previous item
 */
private mixed **stackPrev(mixed **stack)
{
    string key;
    mixed *ref;
    object node;

    key = stack[0][KEY];

    do {
	ref = stack[0];
	stack = stack[1 ..];

	if (!(node=(sizeof(stack) != 0) ? stack[0][VALUE] : root) ||
	    !(ref=node->refIndex(accessKey, ref[CHANGES], ref[INDEX] - 1))) {
	    /*
	     * reference outdated: fall back to search by key
	     */
	    stack = stackKey(key);
	    if (stack) {
		continue;
	    }
	    return nil;
	}

	if (ref[VALUE] != nil || ref[KEY]) {
	    /*
	     * not out of range
	     */
	    for (;;) {
		stack = ({ ref }) + stack;
		if (ref[KEY]) {
		    /* leaf */
		    return stack;
		}
		ref = ref[VALUE]->refLast(accessKey);
	    }
	}
    } while (sizeof(stack) != 0);
}

/*
 * reset an iterator
 */
mixed iteratorStart(mixed from, mixed to)
{
    string first, last;
    mixed **stack;

    first = from;
    last = to;
    if (first && last && first > last) {
	/*
	 * backwards
	 */
	stack = stackKey(first);
	if (stack && stack[0][KEY] != first) {
	    stack = stackPrev(stack);
	}
	return ({ stack, last, TRUE });
    }

    return ({ (first) ? stackKey(first) : stackFirst(), last, FALSE });
}

/*
 * return the next state for an iterator
 */
mixed *iteratorNext(mixed state)
{
    mixed **stack, value;
    string last, key;
    int reverse;

    ({ stack, last, reverse }) = state;
    if (stack) {
	key = stack[0][KEY];
	value = stack[0][VALUE];
	if (key) {
	    if (!reverse) {
		if (!last || key <= last) {
		    return ({
			({ stackNext(stack), last, reverse }),
			({ key, value })
		    });
		}
	    } else if (key >= last) {
		return ({
		    ({ stackPrev(stack), last, reverse }),
		    ({ key, value })
		});
	    }
	}
    }

    return ({ ({ nil, last, reverse }), ({ nil, nil }) });
}

/*
 * check whether an iterator is at its end
 */
int iteratorEnd(mixed state)
{
    mixed **stack;
    string last, key;
    int reverse;

    ({ stack, last, reverse }) = state;
    if (!stack) {
	return TRUE;
    }
    key = stack[0][KEY];
    if (!key) {
	return TRUE;
    }

    return (reverse) ? (key < last) : (last && key > last);
}
