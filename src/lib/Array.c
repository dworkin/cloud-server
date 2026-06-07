# include <Iterator.h>
# include <Array.h>
# include <type.h>

inherit Iterable;


# define ASIZE		32767
# define AOP		"::op"
# define ADD		0
# define SLICE		1
# define SUB		2
# define INTERSECT	3
# define SETADD		4
# define SETXADD	5

private mixed **elements;	/* sub arrays */
private int size;		/* total size */

/*
 * create array
 */
static void create(mixed arg)
{
    int sz, i;

    switch (typeof(arg)) {
    case T_INT:
	/*
	 * allocate with size
	 */
	size = arg;
	if (size < 0 || size > ASIZE * ASIZE) {
	    error("Invalid Array size");
	}
	if (size == 0) {
	    elements = ({ });
	} else {
	    sz = (size - 1) / ASIZE;
	    elements = allocate(sz + 1);
	    for (i = 0; i < sz; i++) {
		elements[i] = allocate(ASIZE);
	    }
	    elements[i] = allocate((size - 1) % ASIZE + 1);
	}
	break;

    case T_ARRAY:
	/*
	 * copy from a LPC array
	 */
	elements = ({ arg[..] });
	size = sizeof(arg);
	break;

    case T_OBJECT:
	/*
	 * constructed by an Array operation
	 */
	if (arg <- Array) {
	    ({ elements, size }) = arg->build();
	    break;
	}
	/* fall through */
    default:
	error("Bad Array initializer");
    }
}

/*
 * Array index
 */
static mixed operator[] (int index)
{
    if (index < 0 || index >= size) {
	error("Array index out of range");
    }
    return elements[index / ASIZE][index % ASIZE];
}

/*
 * Array indexed assignment
 */
static void operator[]= (int index, mixed element)
{
    if (index < 0 || index >= size) {
	error("Array index out of range");
    }
    elements[index / ASIZE][index % ASIZE] = element;
}

/*
 * new Array as result of an operation
 */
private atomic Array operation(int op, mixed arg)
{
    Array a;

    tls_set(AOP, ({ op, arg }));
    a = new Array(this_object());
    tls_set(AOP, nil);

    return a;
}

/*
 * e1[to .. to + size - 1] =  e2[from .. from + size - 1]
 */
private void copy(mixed **e1, int to, mixed **e2, int from, int size)
{
    int f1, f2, t1, t2, rsize;
    mixed *chunk;

    f1 = from / ASIZE;
    f2 = from % ASIZE;
    t1 = to / ASIZE;
    t2 = to % ASIZE;

    if (t2 != 0) {
	chunk = e1[t1][.. t2 - 1];
	rsize = ASIZE - t2;
	if (size <= rsize) {
	    /*
	     * all in one chunk
	     */
	    if (f2 + size <= ASIZE) {
		e1[t1] = chunk + e2[f1][f2 .. f2 + size - 1];
	    } else {
		e1[t1] = chunk + e2[f1][f2 ..] +
			 e2[f1 + 1][.. f2 + size - 1 - ASIZE];
	    }
	    return;
	}

	/*
	 * lead chunk
	 */
	if (t2 > f2) {
	    e1[t1] = chunk + e2[f1][f2 .. f2 + rsize - 1];
	    f2 += rsize;
	} else {
	    e1[t1] = chunk + e2[f1][f2 ..] +
		     e2[f1 + 1][.. f2 + rsize - 1 - ASIZE];
	    f1++;
	    f2 = rsize - f2;
	}
	size -= rsize;
	t1++;
    }

    /*
     * middle chunks
     */
    if (f2 == 0) {
	while (size >= ASIZE) {
	    e1[t1++] = e2[f1++][..];
	    size -= ASIZE;
	}
    } else {
	while (size >= ASIZE) {
	    e1[t1++] = e2[f1][f2 ..] + e2[f1 + 1][.. f2 - 1];
	    f1++;
	    size -= ASIZE;
	}
    }

    /*
     * tail chunk
     */
    if (size != 0) {
	if (f2 + size <= ASIZE) {
	    e1[t1] = e2[f1][f2 .. f2 + size - 1];
	} else {
	    e1[t1] = e2[f1][f2 ..] + e2[f1 + 1][.. f2 + size - 1 - ASIZE];
	}
    }
}

/*
 * add two Arrays
 */
private mixed *add(mixed **e1, int sz1, mixed **e2, int sz2)
{
    mixed **e;

    e = allocate((sz1 + sz2 - 1) / ASIZE + 1);
    copy(e, 0, e1, 0, sz1);
    copy(e, sz1, e2, 0, sz2);

    return ({ e, sz1 + sz2 });
}

/*
 * subtract one Array from another
 */
private mixed *sub(mixed **e1, mixed **e2)
{
    mixed **e, *arr;
    int sz, sz1, sz2, i, j, asz, last, lsz;

    e = ({ });
    sz = 0;
    for (sz1 = sizeof(e1), sz2 = sizeof(e2), i = 0; i < sz1; i++) {
	arr = e1[i];
	for (j = 0; j < sz2; j++) {
	    arr -= e2[j];
	}

	asz = sizeof(arr);
	if (asz != 0) {
	    last = sizeof(e) - 1;
	    if (last < 0) {
		e = ({ arr });
		sz = asz;
	    } else {
		lsz = sizeof(e[last]);
		if (lsz == ASIZE) {
		    e += ({ arr });
		} else if (lsz + sizeof(arr) <= ASIZE) {
		    e[last] += arr;
		} else {
		    lsz = ASIZE - lsz;
		    e[last] += arr[.. lsz - 1];
		    e += ({ arr[lsz ..] });
		}

		sz += asz;
	    }
	}
    }

    return ({ e, sz });
}

/*
 * build a new Array
 */
mixed *build()
{
    int op, from, to, sz;
    mixed arg, **e;

    ({ op, arg }) = tls_get(AOP);
    switch (op) {
    case ADD:
	return add(elements, size, arg->elements(), arg->size());

    case SLICE:
	({ from, to }) = arg;
	sz = to - from + 1;
	e = allocate((sz - 1) / ASIZE + 1);
	copy(e, 0, elements, from, sz);
	return ({ e, sz });

    case SUB:
	return sub(elements, arg->elements());

    case INTERSECT:
	return sub(elements, sub(elements, arg->elements())[0]);

    case SETADD:
	return add(elements, size, sub(arg->elements(), elements)...);

    case SETXADD:
	e = arg->elements();
	return add((sub(elements, e) + sub(e, elements))...);
    }
}

/*
 * Array addition
 */
static Array operator+ (Array a)
{
    if (!a) {
	error("Bad argument 2 for addition");
    }
    return operation(ADD, a);
}

/*
 * Array subrange
 */
static Array operator[..] (mixed from, mixed to)
{
    int f1;

    if (from == nil) {
	from = 0;
    } else if (typeof(from) != T_INT || from < 0 || from >= size) {
	error("Invalid Array subrange");
    }
    if (to == nil) {
	to = size - 1;
    } else if (typeof(to) != T_INT || to < from - 1 || to >= size) {
	error("Invalid Array subrange");
    }
    if (to == from - 1) {
	return new Array(0);
    }

    f1 = from / ASIZE;
    if (f1 == to / ASIZE) {
	return new Array(elements[f1][from % ASIZE .. to % ASIZE]);
    }

    return operation(SLICE, ({ from, to }));
}

/*
 * Array subtraction
 */
static Array operator- (Array a)
{
    if (!a) {
	error("Bad argument 2 for subtraction");
    }
    return operation(SUB, a);
}

/*
 * Array intersection
 */
static Array operator& (Array a)
{
    if (!a) {
	error("Bad argument 2 for intersection");
    }
    return operation(INTERSECT, a);
}

/*
 * Array setadd
 */
static Array operator| (Array a)
{
    if (!a) {
	error("Bad argument 2 for setadd");
    }
    return operation(SETADD, a);
}

/*
 * Array setxadd
 */
static Array operator^ (Array a)
{
    if (!a) {
	error("Bad argument 2 for setxadd");
    }
    return operation(SETXADD, a);
}


/*
 * reset an iterator
 */
mixed iteratorStart(mixed from, mixed to)
{
    if (from == nil) {
	from = 0;
    } else if (typeof(from) != T_INT || from < 0 || from >= size) {
	error("Invalid Array subrange");
    }
    if (to == nil) {
	to = size - 1;
    } else if (typeof(to) != T_INT || to < from - 1 || to >= size) {
	error("Invalid Array subrange");
    }

    return ({ from - 1, to });
}

/*
 * return the next state for an iterator
 */
mixed *iteratorNext(mixed state)
{
    int index, end;

    ({ index, end }) = state;
    if (index == end) {
	return ({ ({ index, end }), nil });
    }
    return ({ ({ ++index, end }), elements[index / ASIZE][index % ASIZE] });
}

/*
 * check if an iterator is at its end
 */
int iteratorEnd(mixed state)
{
    return (state[0] == state[1]);
}


/*
 * access elements
 */
mixed **elements()
{
    if (previous_program() == OBJECT_PATH(Array)) {
	return elements;
    }
}

int size()		{ return size; }
