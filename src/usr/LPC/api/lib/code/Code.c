# include <String.h>
# include "Code.h"

inherit "/lib/util/ascii";


# define LINE	"::line"
# define EMIT	"::emit"

private int lineNumber;		/* line number of this code fragment */

/*
 * initialize code fragment emitter
 */
static void create(varargs int line)
{
    lineNumber = line;
}

/*
 * start emitting code
 */
static void start()
{
    tls_set(LINE, 1);
    tls_set(EMIT, new StringBuffer);
}

/*
 * emit code
 */
void code();

/*
 * finish emitting code
 */
static StringBuffer end()
{
    StringBuffer buffer;

    buffer = tls_get(EMIT);
    buffer->append("\n");
    return buffer;
}

/*
 * return line number of this code fragment
 */
static int line()
{
    return lineNumber;
}

/*
 * emit a code fragment
 */
static void emit(string code, varargs int line)
{
    if (line != 0) {
	lineNumber = line;
    }
    line = tls_get(LINE);
    if (lineNumber != 0 && lineNumber != line) {
	if (lineNumber < line || lineNumber  - line > 10) {
	    code = "\n#line " + lineNumber + "\n" + code;
	} else {
	    code = "\n\n\n\n\n\n\n\n\n\n"[.. lineNumber - line - 1] + code;
	}
	tls_set(LINE, lineNumber);
    }
    lineNumber = 0;

    tls_get(EMIT)->append(code);
}

/*
 * emit a floating point number, with full accuracy
 */
static void emitFloat(float flt, varargs int line)
{
    emit(float2string(flt), line);
}

/*
 * emit a string constant
 */
static void emitString(string str, varargs int line)
{
    emit("\"" + stringify(str) + "\"", line);
}
