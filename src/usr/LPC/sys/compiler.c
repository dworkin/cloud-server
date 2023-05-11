# include "Code.h"
# include "expression.h"
# include "compiler.h"

inherit LPC_PARSE_UTIL;


# define LINE	"::line"

object stringParser;	/* separate parser for strings */
object lineParser;	/* #line parser */

/*
 * initialize LPC parser
 */
static void create()
{
    stringParser = find_object(LPC_STRING_PARSER);
    lineParser = find_object(LPC_LINE_PARSER);
}

/*
 * compile LPC source code to program
 */
LPCCode compile(string source)
{
    mixed *parsed;

    tls_set(LINE, 0);

    parsed = parse_string("\
ident = /[a-zA-Z_][a-zA-Z_0-9]*/					\
simple_string = /\"[^\"\\\\\n]*\"/					\
complex_string = /\"([^\"\\\\\n]|\\\\.)+\"/				\
simple_char = /'[^'\\\\\n]+'/						\
complex_char = /'([^'\\\\\n]|\\\\.)+'/					\
decimal = /[1-9][0-9]*/							\
octal = /0[0-7]*/							\
hexadecimal = /0[xX][a-fA-F0-9]+/					\
float = /[0-9]+\\.[0-9]+([eE][-+]?[0-9]+)?/				\
float = /\\.[0-9]+([eE][-+]?[0-9]+)?/					\
float = /[0-9]+[eE][-+]?[0-9]+/						\
spaces = /([ \t\v\f\r]|\\/\\*([^*\n]*\\*+[^/*\n])*[^*\n]*\\*+\\/)+/	\
newline = /\n([ \t\v\f\r]|\\/\\*([^*\n]*\\*+[^/*\n])*[^*\n]*\\*+\\/)*/	\
newlines = /(\\/\\*([^*]*\\*+[^/*])*[^*]*\\*+\\/[ \t\v\f\r]*)+/		\
line = /\n([ \t\v\f\r]|\\/\\*([^*\n]*\\*+[^/*\n])*[^*\n]*\\*+\\/)*#[^\n]*/										\
line = /(\\/\\*([^*]*\\*+[^/*])*[^*]*\\*+\\/[ \t\v\f\r]*)+#[^\n]*/	" +
"\
Program: Inherits TopLevelDecls _			? program	\
Inherits: InheritList					? list		\
InheritList:								\
InheritList: InheritList Inherit					\
Inherit: OptPrivate _ 'inherit' OptLabel OptObject StringExp _ ';'	\
							? inh		\
OptPrivate:						? false		\
OptPrivate: _ 'private'					? true		\
OptLabel:						? opt		\
OptLabel: _ ident							\
OptObject:								\
OptObject: _ 'object'					? empty		\
TopLevelDecls: TopLevelDeclList				? list		\
TopLevelDeclList:							\
TopLevelDeclList: TopLevelDeclList TopLevelDecl				\
TopLevelDecl: DataDecl							\
TopLevelDecl: FuncDecl							" +
"\
DataDecl: ClassType Dcltrs _ ';'			? dataDecl	\
FuncDecl: ClassType FunctionDcltr CompoundStmt		? functionDecl	\
FuncDecl: Class FunctionName _ '(' Formals _ ')' CompoundStmt		\
							? voidDecl	\
Formals:						? noArguments	\
Formals: _ 'void'					? noArguments	\
Formals: FormalList					? arguments	\
Formals: FormalList _ '...'				? ellipsis	\
FormalList: Formal							\
FormalList: FormalList _ ',' Formal					\
Formal: ClassType DataDcltr				? formal	\
Formal: _ ident						? formalMixed	" +
"\
ClassType: ClassSpecList _ TypeSpec			? classType	\
ClassType: ClassSpecList _ 'object' ListExp		? classTypeName	\
Class: ClassSpecList					? classType	\
ClassSpecList:								\
ClassSpecList: ClassSpecList _ ClassSpec				\
ClassSpec: 'private'							\
ClassSpec: 'static'							\
ClassSpec: 'atomic'							\
ClassSpec: 'nomask'							\
ClassSpec: 'varargs'							\
TypeSpec: 'int'								\
TypeSpec: 'float'							\
TypeSpec: 'string'							\
TypeSpec: 'object'							\
TypeSpec: 'mapping'							\
TypeSpec: 'mixed'							\
TypeSpec: 'void'							\
DataDcltr: Stars _ ident						\
Stars: StarList						? count		\
StarList:								\
StarList: StarList _ '*'						" +
"\
FunctionName: _ ident							\
FunctionName: _ Operator				? concat	\
Operator: 'operator' _ '+'						\
Operator: 'operator' _ '-'						\
Operator: 'operator' _ '*'						\
Operator: 'operator' _ '/'						\
Operator: 'operator' _ '%'						\
Operator: 'operator' _ '&'						\
Operator: 'operator' _ '^'						\
Operator: 'operator' _ '|'						\
Operator: 'operator' _ '<'						\
Operator: 'operator' _ '>'						\
Operator: 'operator' _ '>='						\
Operator: 'operator' _ '<='						\
Operator: 'operator' _ '<<'						\
Operator: 'operator' _ '>>'						\
Operator: 'operator' _ '~'						\
Operator: 'operator' _ '++'						\
Operator: 'operator' _ '--'						\
Operator: 'operator' _ '[' _ ']'					\
Operator: 'operator' _ '[' _ ']' _ '='					\
Operator: 'operator' _ '[' _ '..' _ ']'					" +
"\
FunctionDcltr: Stars FunctionName _ '(' Formals _ ')'			\
Dcltr: DataDcltr					? list		\
Dcltr: FunctionDcltr					? list		\
Dcltrs: ListDcltr					? noCommaList	\
ListDcltr: Dcltr							\
ListDcltr: ListDcltr _ ',' Dcltr					\
Locals: ListLocal					? list		\
ListLocal:								\
ListLocal: ListLocal DataDecl						\
Exception: ident							\
Exception: '...'							\
ListStmt:								\
ListStmt: ListStmt Stmt					? listStmt	" +
"\
IfStmt: _ 'if' _ '(' ListExp _ ')' Stmt			? ifStmt	\
Stmt: BStmt								\
Stmt: IfStmt								\
Stmt: Entries IfStmt					? entryStmt	\
BStmt: EStmt								\
BStmt: Entries EStmt					? entryStmt	\
EStmt: ListExp _ ';'					? expStmt	\
EStmt: CompoundStmt							\
EStmt: _ 'if' _ '(' ListExp _ ')' BStmt _ 'else' Stmt	? ifElseStmt	\
EStmt: _ 'do' Stmt _ 'while' _ '(' ListExp _ ')' _ ';'	? doWhileStmt	\
EStmt: _ 'while' _ '(' ListExp _ ')' Stmt		? whileStmt	\
EStmt: _ 'for' _ '(' OptListExp _ ';'					\
		     OptListExp _ ';'					\
		     OptListExp _ ')' Stmt		? forStmt	\
EStmt: _ 'rlimits' _ '(' ListExp _ ';'					\
			 ListExp _ ')' CompoundStmt	? rlimitsStmt	\
EStmt: _ 'try' CompoundStmt						\
       _ 'catch' _ '(' _ Exception _ ')' CompoundStmt	? tryCatchStmt	\
EStmt: _ 'catch' CompoundStmt _ ':' Stmt		? catchErrStmt	\
EStmt: _ 'catch' CompoundStmt				? catchStmt	" +
"\
EStmt: _ 'switch' _ '(' ListExp _ ')' CompoundStmt	? switchStmt	\
EStmt: _ 'goto' _ ident _ ';'				? gotoStmt	\
EStmt: _ 'break' _ ';'					? breakStmt	\
EStmt: _ 'continue' _ ';'				? continueStmt	\
EStmt: _ 'return' ListExp _ ';'				? returnExpStmt	\
EStmt: _ 'return' _ ';'					? returnStmt	\
EStmt: _ ';'						? emptyStmt	\
Entries: _ Entry							\
Entries: Entries _ Entry						\
Entry: 'case' Exp _ ':'					? caseEntry	\
Entry: 'case' Exp _ '..' Exp _ ':'			? rangeEntry	\
Entry: 'default' _ ':'					? defaultEntry	\
Entry: ident _ ':'					? labelEntry	\
CompoundStmt: _ '{' Locals ListStmt _ '}'		? compoundStmt	" +
"\
FunctionCall: FunctionName						\
FunctionCall: _ '::' FunctionName					\
FunctionCall: _ ident _ '::' FunctionName				\
String: _ simple_string					? simpleString	\
String: _ complex_string				? complexString	\
CompositeString: StringExp						\
CompositeString: CompositeString _ '+' StringExp	? stringExp	\
StringExp: String							\
StringExp: _ '(' CompositeString _ ')'			? parsed_1_	" +
"\
Exp1: _ decimal						? expIntDec	\
Exp1: _ octal						? expIntOct	\
Exp1: _ hexadecimal					? expIntHex	\
Exp1: _ simple_char					? simpleChar	\
Exp1: _ complex_char					? complexChar	\
Exp1: _ float						? expFloat	\
Exp1: _ 'nil'						? expNil	\
Exp1: String								\
Exp1: _ '(' _ '{' OptArgListComma _ '}' _ ')'		? expArray	\
Exp1: _ '(' _ '[' OptAssocListComma _ ']' _ ')'		? expMapping	\
Exp1: _ ident						? expVar	\
Exp1: _ '::' _ ident					? expGlobalVar	\
Exp1: _ '(' _ ListExp _ ')'				? parsed_1_	\
Exp1: FunctionCall _ '(' OptArgList _ ')'		? expFuncall	\
Exp1: _ 'catch' _ '(' ListExp _ ')'			? expCatch	\
Exp1: _ 'new' OptObject StringExp			? expNew1	\
Exp1: _ 'new' OptObject StringExp _ '(' OptArgList _ ')'? expNew2	\
Exp1: Exp2 _ '->' _ ident _ '(' OptArgList _ ')'	? expCallOther	\
Exp1: Exp2 _ '<-' StringExp				? expInstance	" +
"\
Exp2: Exp1								\
Exp2: Exp2 _ '[' ListExp _ ']'				? expIndex	\
Exp2: Exp2 _ '[' OptListExp _ '..' OptListExp _ ']'	? expRange	" +
"\
PostfixExp: Exp2							\
PostfixExp: PostfixExp _ '++'				? expPostIncr	\
PostfixExp: PostfixExp _ '--'				? expPostDecr	" +
"\
PrefixExp: PostfixExp							\
PrefixExp: _ '++' CastExp				? expPreIncr	\
PrefixExp: _ '--' CastExp				? expPreDecr	\
PrefixExp: _ '+' CastExp				? expPlus	\
PrefixExp: _ '-' CastExp				? expMinus	\
PrefixExp: _ '!' CastExp				? expNot	\
PrefixExp: _ '~' CastExp				? expNegate	" +
"\
CastExp: PrefixExp							\
CastExp: _ '(' ClassType Stars _ ')' CastExp		? expCast	" +
"\
MultExp: CastExp							\
MultExp: MultExp _ '*' CastExp				? expMult	\
MultExp: MultExp _ '/' CastExp				? expDiv	\
MultExp: MultExp _ '%' CastExp				? expMod	" +
"\
AddExp: MultExp								\
AddExp: AddExp _ '+' MultExp				? expAdd	\
AddExp: AddExp _ '-' MultExp				? expSub	" +
"\
ShiftExp: AddExp							\
ShiftExp: ShiftExp _ '<<' AddExp			? expLShift	\
ShiftExp: ShiftExp _ '>>' AddExp			? expRShift	" +
"\
RelExp: ShiftExp							\
RelExp: RelExp _ '<' ShiftExp				? expLess	\
RelExp: RelExp _ '>' ShiftExp				? expGreater	\
RelExp: RelExp _ '<=' ShiftExp				? expLessEq	\
RelExp: RelExp _ '>=' ShiftExp				? expGreaterEq	" +
"\
EquExp: RelExp								\
EquExp: EquExp _ '==' RelExp				? expEqual	\
EquExp: EquExp _ '!=' RelExp				? expUnequal	" +
"\
BitandExp: EquExp							\
BitandExp: BitandExp _ '&' EquExp			? expAnd	" +
"\
BitxorExp: BitandExp							\
BitxorExp: BitxorExp _ '^' BitandExp			? expXor	" +
"\
BitorExp: BitxorExp							\
BitorExp: BitorExp _ '|' BitxorExp			? expOr		" +
"\
AndExp: BitorExp							\
AndExp: AndExp _ '&&' BitorExp				? expLand	" +
"\
OrExp: AndExp								\
OrExp: OrExp _ '||' AndExp				? expLor	" +
"\
CondExp: OrExp								\
CondExp: OrExp _ '?' ListExp _ ':' CondExp		? expQuest	" +
"\
Exp: CondExp								\
Exp: CondExp _ '=' Exp					? expAssign	\
Exp: CondExp _ '+=' Exp					? expAsgnAdd	\
Exp: CondExp _ '-=' Exp					? expAsgnSub	\
Exp: CondExp _ '*=' Exp					? expAsgnMult	\
Exp: CondExp _ '/=' Exp					? expAsgnDiv	\
Exp: CondExp _ '%=' Exp					? expAsgnMod	\
Exp: CondExp _ '<<=' Exp				? expAsgnLShift	\
Exp: CondExp _ '>>=' Exp				? expAsgnRShift	\
Exp: CondExp _ '&=' Exp					? expAsgnAnd	\
Exp: CondExp _ '^=' Exp					? expAsgnXor	\
Exp: CondExp _ '|=' Exp					? expAsgnOr	" +
"\
ListExp: Exp								\
ListExp: ListExp _ ',' Exp				? expComma	\
OptListExp:						? opt		\
OptListExp: ListExp							" +
"\
ArgList: Exp								\
ArgList: ArgList _ ',' Exp						\
OptArgList:						? noArguments	\
OptArgList: ArgList					? arguments	\
OptArgList: ArgList _ '...'				? ellipsis	\
OptArgListComma:					? list		\
OptArgListComma: ArgList				? noCommaList	\
OptArgListComma: ArgList _ ','				? noCommaList	" +
"\
AssocPair: Exp _ ':' Exp				? parsed_0_2_	\
AssocList: AssocPair							\
AssocList: AssocList _ ',' AssocPair					\
OptAssocListComma:					? list		\
OptAssocListComma: AssocList				? noCommaList	\
OptAssocListComma: AssocList _ ','			? noCommaList	" +
"\
_:									\
_: _ spaces						? empty		\
_: _ newline						? newline	\
_: _ newlines						? newlines	\
_: _ line						? line		",
			  "\n" + source);
    return (parsed) ? parsed[0] : nil;
}

/*
 * ({ ({ }), ({ }) })
 */
static mixed *program(mixed *parsed)
{
    return ({ new LPCProgram(parsed[0], parsed[1]) });
}

/*
 * ({ private, "inherit", label, LPCExpression })
 */
static mixed *inh(mixed *parsed)
{
    return ({ new LPCInherit(parsed[0], parsed[2], parsed[3]) });
}

/*
 * ({ "private", "int" })
 */
static mixed *classType(mixed *parsed)
{
    return ({ implode(parsed, " "), nil, tls_get(LINE) });
}

/*
 * ({ "private", "object", LPCExpression })
 */
static mixed *classTypeName(mixed *parsed)
{
    int sz;

    sz = sizeof(parsed);
    return ({ implode(parsed[.. sz - 2], " "), parsed[sz - 1], tls_get(LINE) });
}

/*
 * ({ parsed1, ",", parsed2 })
 */
static mixed *noCommaList(mixed *parsed)
{
    return ({ parsed - ({ "," }) });
}

/*
 * ({ "private int", nil, 100, ({ ({ 0, "a" }), ({ 1, "b" }) }) })
 */
static mixed *dataDecl(mixed *parsed)
{
    mixed *decls;
    int i, sz;
    LPCType type;

    decls = parsed[3];
    for (i = 0, sz = sizeof(decls); i < sz; i++) {
	type = new LPCType(parsed[0], parsed[1], decls[i][0], parsed[2]);
	decls[i] = (sizeof(decls[i]) == 2) ?
		    new LPCVariable(type, decls[i][1]) :
		    new LPCFunction(type, decls[i][1], decls[i][3], decls[i][4],
				    nil);
    }
    return decls;
}

/*
 * ({ })
 */
static mixed *noArguments(mixed *parsed)
{
    return ({ ({ }), FALSE });
}

/*
 * ({ parsed1, ",", parsed2 })
 */
static mixed *arguments(mixed *parsed)
{
    return ({ parsed - ({ "," }), FALSE });
}

/*
 * ({ parsed1, ",", parsed2, "..." })
 */
static mixed *ellipsis(mixed *parsed)
{
    return ({ parsed[.. sizeof(parsed) - 2] - ({ "," }), TRUE });
}

/*
 * ({ "private int", nil, 100, 0, "a" })
 */
static mixed *formal(mixed *parsed)
{
    return ({
	new LPCDeclaration(new LPCType(parsed[0], parsed[1], parsed[3],
				       parsed[2]),
			   parsed[4])
    });
}

/*
 * ({ "a" })
 */
static mixed *formalMixed(mixed *parsed)
{
    return ({
	new LPCDeclaration(new LPCType("mixed", nil, 0, tls_get(LINE)), parsed[0])
    });
}

/*
 * ({ "private int", nil, 100, 0, "func", '(' ({ }), FALSE, ')', LPCStmtBlock })
 */
static mixed *functionDecl(mixed *parsed)
{
    return ({
	new LPCFunction(new LPCType(parsed[0], parsed[1], parsed[3], parsed[2]),
			parsed[4], parsed[6], parsed[7], parsed[9])
    });
}

/*
 * ({ "private", nil, 100, "func", '(', ({ }), FALSE, ')', LPCStmtBlock })
 */
static mixed *voidDecl(mixed *parsed)
{
    return ({
	new LPCFunction(new LPCType(parsed[0], parsed[1], 0, parsed[2]),
			parsed[3], parsed[5], parsed[6], parsed[8])
    });
}

/*
 * ({ firstStmt, lastStmt, additionalStmt })
 */
static mixed *listStmt(mixed *parsed)
{
    if (sizeof(parsed) == 1) {
	return ({ parsed[0], parsed[0] });
    }
    parsed[1]->setNext(parsed[2]);
    return ({ parsed[0], parsed[2] });
}

/*
 * ({ LPCExpression, ";" })
 */
static mixed *expStmt(mixed *parsed)
{
    return ({ new LPCStmtExp(parsed[0]) });
}

/*
 * ({ ";" })
 */
static mixed *emptyStmt(mixed *parsed)
{
    return ({ new LPCStatement });
}

/*
 * ({ "{" ({ }), firstStmt, lastStmt, "}" })
 */
static mixed *compoundStmt(mixed *parsed)
{
    return ({
	new LPCStmtBlock(parsed[1], ((sizeof(parsed) == 3) ? nil : parsed[2]))
    });
}

/*
 * ({ "1234" })
 */
static mixed *expIntDec(mixed *parsed)
{
    return ({ new LPCExpression((int) parsed[0]) });
}

/*
 * ({ "0123" })
 */
static mixed *expIntOct(mixed *parsed)
{
    return ({ new LPCExpression(stringParser->octalInt(parsed[0])) });
}

/*
 * ({ "0x123" })
 */
static mixed *expIntHex(mixed *parsed)
{
    return ({ new LPCExpression(stringParser->hexadecimalInt(parsed[0])) });
}

/*
 * ({ "12.34" })
 */
static mixed *expFloat(mixed *parsed)
{
    return ({ new LPCExpression((float) parsed[0]) });
}

/*
 * ({ "\"abc\"" })
 */
static mixed *simpleString(mixed *parsed)
{
    string str;

    str = parsed[0];
    str = str[1 .. strlen(str) - 2];
    return ({ new LPCExpression(str) });
}

/*
 * ({ "\"ab\\nc\"" })
 */
static mixed *complexString(mixed *parsed)
{
    string str;

    str = parsed[0];
    str = str[1 .. strlen(str) - 2];
    return ({ new LPCExpression(stringParser->parse(str)) });
}

/*
 * ({ CompositeString, "+", StringExp })
 */
static mixed *stringExp(mixed *parsed)
{
    return ({ new LPCExpression(parsed[0]->value() + parsed[2]->value()) });
}

/*
 * ({ "'a'" })
 */
static mixed *simpleChar(mixed *parsed)
{
    return ({ new LPCExpression(parsed[0][1]) });
}

/*
 * ({ "'\\n'" })
 */
static mixed *complexChar(mixed *parsed)
{
    string str;

    str = parsed[0];
    str = str[1 .. strlen(str) - 2];
    return ({ new LPCExpression(stringParser->parse(str)[0]) });
}

/*
 * ({ "nil" })
 */
static mixed *expNil(mixed *parsed)
{
    return ({ new LPCExpression(nil) });
}

/*
 * ({ "(", "{", ({ }), "}", ")" })
 */
static mixed *expArray(mixed *parsed)
{
    return ({ new LPCExpArray(parsed[2]) });
}

/*
 * ({ "(", "[", ({ }), "]", ")" })
 */
static mixed *expMapping(mixed *parsed)
{
    return ({ new LPCExpMapping(parsed[2]) });
}

/*
 * ({ "a" })
 */
static mixed *expVar(mixed *parsed)
{
    return ({ new LPCExpVar(parsed[0]) });
}

/*
 * ({ "::", "a" })
 */
static mixed *expGlobalVar(mixed *parsed)
{
    return ({ new LPCExpGlobalVar(parsed[1]) });
}

/*
 * ({ "func", "(", ({ }), FALSE, ")" })
 */
static mixed *expFuncall(mixed *parsed)
{
    switch (sizeof(parsed)) {
    case 5:
	/* func() */
	return ({ new LPCExpFuncall(parsed[0], parsed[2], parsed[3]) });

    case 6:
	/* ::func() */
	return ({ new LPCExpInhFuncall(nil, parsed[1], parsed[3], parsed[4]) });

    case 7:
	/* label::func() */
	return ({
	    new LPCExpInhFuncall(parsed[0], parsed[2], parsed[4], parsed[5])
	});
    }
}

/*
 * ({ "catch", "(", LPCExpression, ")" })
 */
static mixed *expCatch(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_CATCH, parsed[2]) });
}

/*
 * ({ "new", LPCExpression })
 */
static mixed *expNew1(mixed *parsed)
{
    return ({ new LPCExpFuncall("new_object", ({ parsed[1] }), FALSE) });
}

/*
 * ({ "new", LPCExpression, "(", ({ }), FALSE, ")" })
 */
static mixed *expNew2(mixed *parsed)
{
    return ({
	new LPCExpFuncall("new_object", ({ parsed[1] }) + parsed[3],
			  parsed[4])
    });
}

/*
 * ({ LPCExpression, "->", "func", "(", ({ }), FALSE, ")" })
 */
static mixed *expCallOther(mixed *parsed)
{
    return ({
	new LPCExpFuncall("call_other",
			  ({
			      parsed[0], new LPCExpression(parsed[2])
			  }) + parsed[4],
			  parsed[5])
    });
}

/*
 * ({ LPCExpression, "<-", LPCExpression })
 */
static mixed *expInstance(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_INSTANCEOF, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "[", LPCExpression, "]" })
 */
static mixed *expIndex(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_INDEX, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "[", LPCExpression, "..", LPCExpression, "]" })
 */
static mixed *expRange(mixed *parsed)
{
    return ({ new LPCExp3(LPC_EXP_RANGE, parsed[0], parsed[2], parsed[4]) });
}

/*
 * ({ LPCExpression, "++" })
 */
static mixed *expPostIncr(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_POST_INCR, parsed[0]) });
}

/*
 * ({ LPCExpression, "--" })
 */
static mixed *expPostDecr(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_POST_DECR, parsed[0]) });
}

/*
 * ({ "++", LPCExpression })
 */
static mixed *expPreIncr(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_PRE_INCR, parsed[1]) });
}

/*
 * ({ "--", LPCExpression })
 */
static mixed *expPreDecr(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_PRE_DECR, parsed[1]) });
}

/*
 * ({ "+", LPCExpression })
 */
static mixed *expPlus(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_UPLUS, parsed[1]) });
}

/*
 * ({ "-", LPCExpression })
 */
static mixed *expMinus(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_UMIN, parsed[1]) });
}

/*
 * ({ "!", LPCExpression })
 */
static mixed *expNot(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_NOT, parsed[1]) });
}

/*
 * ({ "~", LPCExpression })
 */
static mixed *expNegate(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_NEG, parsed[1]) });
}

/*
 * ({ "(", "private int", nil, 100, 0, ")", LPCExpression })
 */
static mixed *expCast(mixed *parsed)
{
    return ({
	new LPCExpCast(new LPCType(parsed[1], parsed[2], parsed[4], parsed[3]),
		       parsed[6])
    });
}

/*
 * ({ LPCExpression, "*", LPCExpression })
 */
static mixed *expMult(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_MULT, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "/", LPCExpression })
 */
static mixed *expDiv(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_DIV, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "%", LPCExpression })
 */
static mixed *expMod(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_MOD, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "+", LPCExpression })
 */
static mixed *expAdd(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ADD, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "-", LPCExpression })
 */
static mixed *expSub(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_SUB, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "<<", LPCExpression })
 */
static mixed *expLShift(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LSHIFT, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, ">>", LPCExpression })
 */
static mixed *expRShift(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_RSHIFT, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "<", LPCExpression })
 */
static mixed *expLess(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LT, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, ">", LPCExpression })
 */
static mixed *expGreater(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_GT, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "<=", LPCExpression })
 */
static mixed *expLessEq(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LE, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, ">=", LPCExpression })
 */
static mixed *expGreaterEq(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_GE, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "==", LPCExpression })
 */
static mixed *expEqual(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_EQ, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "!=", LPCExpression })
 */
static mixed *expUnequal(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_NE, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "&", LPCExpression })
 */
static mixed *expAnd(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_AND, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "^", LPCExpression })
 */
static mixed *expXor(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_XOR, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "|", LPCExpression })
 */
static mixed *expOr(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_OR, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "&&", LPCExpression })
 */
static mixed *expLand(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LAND, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "||", LPCExpression })
 */
static mixed *expLor(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LOR, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "?", LPCExpression, ":", LPCExpression })
 */
static mixed *expQuest(mixed *parsed)
{
    return ({ new LPCExp3(LPC_EXP_QUEST, parsed[0], parsed[2], parsed[4]) });
}

/*
 * ({ LPCExpression, "=", LPCExpression })
 */
static mixed *expAssign(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "+=", LPCExpression })
 */
static mixed *expAsgnAdd(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_ADD, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "-=", LPCExpression })
 */
static mixed *expAsgnSub(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_SUB, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "*=", LPCExpression })
 */
static mixed *expAsgnMult(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_MULT, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "/=", LPCExpression })
 */
static mixed *expAsgnDiv(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_DIV, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "%=", LPCExpression })
 */
static mixed *expAsgnMod(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_MOD, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "<<=", LPCExpression })
 */
static mixed *expAsgnLShift(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_LSHIFT, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, ">>=", LPCExpression })
 */
static mixed *expAsgnRShift(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_RSHIFT, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "&=", LPCExpression })
 */
static mixed *expAsgnAnd(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_AND, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "^=", LPCExpression })
 */
static mixed *expAsgnXor(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_XOR, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, "|=", LPCExpression })
 */
static mixed *expAsgnOr(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_OR, parsed[0], parsed[2]) });
}

/*
 * ({ LPCExpression, ",", LPCExpression })
 */
static mixed *expComma(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_COMMA, parsed[0], parsed[2]) });
}

/*
 * ({ "if", "(", LPCExpression, ")", LPCStatement })
 */
static mixed *ifStmt(mixed *parsed)
{
    return ({ new LPCStmtConditional(parsed[2], parsed[4], nil) });
}

/*
 * ({ "if", "(", LPCExpression, ")", LPCStatement, 'else', LPCStatement })
 */
static mixed *ifElseStmt(mixed *parsed)
{
    return ({ new LPCStmtConditional(parsed[2], parsed[4], parsed[6]) });
}

/*
 * ({ "do", LPCStatement, "while", "(", LPCExpression, ")", ";" })
 */
static mixed *doWhileStmt(mixed *parsed)
{
    return ({ new LPCStmtDoWhile(parsed[1], parsed[4]) });
}

/*
 * ({ "while", "(", LPCExpression, ")", LPCStatement })
 */
static mixed *whileStmt(mixed *parsed)
{
    return ({ new LPCStmtFor(nil, parsed[2], nil, parsed[4]) });
}

/*
 * ({ "for", "(", LPCExpression, ";", LPCExpression, ";", LPCExpression,
 *    LPCStatement })
 */
static mixed *forStmt(mixed *parsed)
{
    return ({ new LPCStmtFor(parsed[2], parsed[4], parsed[6], parsed[8]) });
}

/*
 * ({ "rlimits", "(", LPCExpression, ";", LPCExpression, ")", LPCStatement })
 */
static mixed *rlimitsStmt(mixed *parsed)
{
    return ({ new LPCStmtRlimits(parsed[2], parsed[4], parsed[6]) });
}

/*
 * ({ "try", LPCBlockStmt, "catch", "(", "...", ")", LPCBlockStmt })
 */
static mixed *tryCatchStmt(mixed *parsed)
{
    return ({ new LPCStmtTryCatch(parsed[1], parsed[4], parsed[6]) });
}

/*
 * ({ "catch", LPCBlockStmt, ":", LPCStatement })
 */
static mixed *catchErrStmt(mixed *parsed)
{
    return ({ new LPCStmtTryCatch(parsed[1], "...", parsed[3]) });
}

/*
 * ({ "catch", LPCBlockStmt })
 */
static mixed *catchStmt(mixed *parsed)
{
    return ({ new LPCStmtTryCatch(parsed[1], "...", nil) });
}

/*
 * ({ "switch", "(", LPCExpression, ")", LPCBlockStmt })
 */
static mixed *switchStmt(mixed *parsed)
{
    return ({ new LPCStmtSwitch(parsed[2], parsed[4]) });
}

/*
 * ({ LPCEntry, LPCEntry, ... LPCEntry, LPCStatement })
 */
static mixed *entryStmt(mixed *parsed)
{
    int size;

    size = sizeof(parsed);
    return ({ new LPCEntryStmt(parsed[.. size - 2], parsed[size - 1]) });
}

/*
 * ({ "case", LPCExpression, ":" })
 */
static mixed *caseEntry(mixed *parsed)
{
    return ({ new LPCCase(parsed[1]) });
}

/*
 * ({ "case", LPCExpression, "..", LPCExpression, ":" })
 */
static mixed *caseRangeEntry(mixed *parsed)
{
    return ({ new LPCCaseRange(parsed[1], parsed[3]) });
}

/*
 * ({ "default", ":" })
 */
static mixed *defaultEntry(mixed *parsed)
{
    return ({ new LPCDefault() });
}

/*
 * ({ "label", ":" })
 */
static mixed *labelEntry(mixed *parsed)
{
    return ({ new LPCLabel(parsed[0]) });
}

/*
 * ({ "goto", "label", ";" })
 */
static mixed *gotoStmt(mixed *parsed)
{
    return ({ new LPCStmtGoto(parsed[1]) });
}

/*
 * ({ "break", ";" })
 */
static mixed *breakStmt(mixed *parsed)
{
    return ({ new LPCStmtBreak });
}

/*
 * ({ "continue", ";" })
 */
static mixed *continueStmt(mixed *parsed)
{
    return ({ new LPCStmtContinue });
}

/*
 * ({ "return", LPCExpression, ";" })
 */
static mixed *returnExpStmt(mixed *parsed)
{
    return ({ new LPCStmtReturnExp(parsed[1]) });
}

/*
 * ({ "return", ";" })
 */
static mixed *returnStmt(mixed *parsed)
{
    return ({ new LPCStmtReturn });
}

/*
 * \n
 */
static mixed *newline(mixed parsed)
{
    tls_set(LINE, tls_get(LINE) + 1);
    return ({ });
}

/*
 * comment containing newlines
 */
static mixed *newlines(mixed *parsed)
{
    tls_set(LINE, tls_get(LINE) + sizeof(explode(parsed[0], "\n")) - 1);
    return ({ });
}

/*
 * #line 100
 */
static mixed *line(mixed *parsed)
{
    tls_set(LINE, lineParser->line(parsed[0]) - 1);
    return ({ });
}
