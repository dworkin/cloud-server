# include <String.h>
# include "code.h"


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
	    code = "#line " + lineNumber + "\n" + code;
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
    string str, str2, str3, exp;
    float flt2;

    str = (string) flt;
    flt2 = (float) str;
    if (sscanf(str, "%se%s", str, exp) != 0) {
	exp = "e" + exp;
    } else {
	exp = "";
    }

    if (flt != flt2) {
	str2 = str;
	str2[strlen(str2) - 1]--;
	str3 = (string) fabs(flt - (float) (str2 + exp));
	if (fabs(flt) > fabs(flt2)) {
	    /* string representation was rounded down */
	    str2 = str3[1 ..];
	    sscanf(str2, ".%s", str2);
	    sscanf(str2, "%se", str2);
	} else {
	    /* string representation was rounded up */
	    str = str2;
	    str2 = str3;
	    sscanf(str2, "%se", str2);
	    if (sscanf(str2, "%s.%s", str2, str3) != 0) {
		str2 += str3;
	    }
	}

	/* extend mantissa */
	if (sscanf(str, "%*s.") == 0) {
	    str += ".";
	}
	str += str2;
    }

    if (exp != "") {
	str += exp;
    } else if (sscanf(str, "%*s.") == 0) {
	str += ".0";
    }
    emit(str, line);
}

/*
 * emit a string constant
 */
static void emitString(string str, varargs int line)
{
    if (sscanf(str, "%*s\\") != 0) {
	str = implode(explode("\\" + str + "\\", "\\"), "\\\\");
    }
    if (sscanf(str, "%*s\"") != 0) {
	str = implode(explode("\"" + str + "\"", "\""), "\\\"");
    }
    if (sscanf(str, "%*s\n") != 0) {
	str = implode(explode("\n" + str + "\n", "\n"), "\\n");
    }
    emit("\"" + str + "\"", line);
}
