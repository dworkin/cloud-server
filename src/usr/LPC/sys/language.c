# include "parser.h"
# include "code.h"
# include "expression.h"

inherit LPC_PARSE_UTIL;


object stringParser;	/* separate parser for strings */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize LPC parser
 */
static void create()
{
    stringParser = compile_object(LPC_STRING_PARSER);
}

/*
 * NAME:	parse()
 * DESCRIPTION:	parse LPC source code, return program
 */
LPCProgram parse(string source)
{
    mixed *parsed;

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
whitespace = /([ \t\v\f\r\n]|\\/\\*([^*]*\\*+[^/*])*[^*]*\\*+\\/)+/	" +
"\
Program: Inherits TopLevelDecls				? program	\
Inherits: InheritList					? list		\
InheritList:								\
InheritList: InheritList Inherit					\
Inherit: OptPrivate 'inherit' OptLabel OptObject Exp ';'		\
							? inh		\
OptPrivate:						? false		\
OptPrivate: 'private'					? true		\
OptLabel:						? opt		\
OptLabel: ident								\
OptObject:								\
OptObject: 'object'					? empty		\
TopLevelDecls: TopLevelDeclList				? list		\
TopLevelDeclList:							\
TopLevelDeclList: TopLevelDeclList TopLevelDecl				\
TopLevelDecl: DataDecl							\
TopLevelDecl: FunctionDecl						" +
"\
DataDecl: ClassType Dcltrs ';'				? dataDecl	\
FunctionDecl: ClassType FunctionDcltr CompoundStmt	? functionDecl	\
FunctionDecl: Class ident '(' Formals ')' CompoundStmt	? voidDecl	\
Formals:						? noArguments	\
Formals: 'void'						? noArguments	\
Formals: FormalList					? arguments	\
Formals: FormalList '...'				? ellipsis	\
FormalList: Formal							\
FormalList: FormalList ',' Formal					\
Formal: ClassType DataDcltr				? formal	\
Formal: ident						? formalMixed	" +
"\
ClassType: ClassSpecList TypeSpec			? classType	\
ClassType: ClassSpecList 'object' ListExp		? classTypeName	\
Class: ClassSpecList					? classType	\
ClassSpecList:								\
ClassSpecList: ClassSpecList ClassSpec					\
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
DataDcltr: Stars ident							\
Stars: StarList						? count		\
StarList:								\
StarList: StarList '*'							" +
"\
FunctionName: ident							\
FunctionName: Operator					? concat	\
Operator: 'operator' '+'						\
Operator: 'operator' '-'						\
Operator: 'operator' '*'						\
Operator: 'operator' '/'						\
Operator: 'operator' '%'						\
Operator: 'operator' '&'						\
Operator: 'operator' '^'						\
Operator: 'operator' '|'						\
Operator: 'operator' '<'						\
Operator: 'operator' '>'						\
Operator: 'operator' '>='						\
Operator: 'operator' '<='						\
Operator: 'operator' '<<'						\
Operator: 'operator' '>>'						\
Operator: 'operator' '~'						\
Operator: 'operator' '++'						\
Operator: 'operator' '--'						\
Operator: 'operator' '[' ']'						\
Operator: 'operator' '[' ']' '='					\
Operator: 'operator' '[' '..' ']'					" +
"\
FunctionDcltr: Stars FunctionName '(' Formals ')'			\
Dcltr: DataDcltr					? list		\
Dcltr: FunctionDcltr					? list		\
Dcltrs: ListDcltr					? noCommaList	\
ListDcltr: Dcltr							\
ListDcltr: ListDcltr ',' Dcltr						\
Locals: ListLocal					? list		\
ListLocal:								\
ListLocal: ListLocal DataDecl						\
ListStmt:								\
ListStmt: ListStmt Stmt					? listStmt	" +
"\
OptElse: 'else' Stmt					? parsed_1_	\
OptElse:						? opt		\
Stmt: ListExp ';'					? expStmt	\
Stmt: CompoundStmt							\
Stmt: 'if' '(' ListExp ')' Stmt OptElse			? ifStmt	\
Stmt: 'do' Stmt 'while' '(' ListExp ')' ';'		? doWhileStmt	\
Stmt: 'while' '(' ListExp ')' Stmt			? whileStmt	\
Stmt: 'for' '(' OptListExp ';' OptListExp ';' OptListExp ')' Stmt	\
							? forStmt	\
Stmt: 'rlimits' '(' ListExp ';' ListExp ')' CompoundStmt		\
							? rlimitsStmt	\
Stmt: 'catch' CompoundStmt ':' Stmt			? catchErrStmt	\
Stmt: 'catch' CompoundStmt				? catchStmt	" +
"\
Stmt: 'switch' '(' ListExp ')' CompoundStmt		? switchStmt	\
Stmt: 'case' Exp ':' Stmt				? caseStmt	\
Stmt: 'case' Exp '..' Exp ':' Stmt			? caseRangeStmt	\
Stmt: 'default' ':' Stmt				? defaultStmt	\
Stmt: ident ':' Stmt					? labelStmt	\
Stmt: 'goto' ident ';'					? gotoStmt	\
Stmt: 'break' ';'					? breakStmt	\
Stmt: 'continue' ';'					? continueStmt	\
Stmt: 'return' ListExp ';'				? returnExpStmt	\
Stmt: 'return' ';'					? returnStmt	\
Stmt: ';'						? emptyStmt	\
CompoundStmt: '{' Locals ListStmt '}'			? compoundStmt	" +
"\
FunctionCall: FunctionName						\
FunctionCall: '::' FunctionName						\
FunctionCall: ident '::' FunctionName					\
String: simple_string					? simpleString	\
String:	complex_string					? complexString " +
"\
Exp1: decimal						? expIntDec	\
Exp1: octal						? expIntOct	\
Exp1: hexadecimal					? expIntHex	\
Exp1: simple_char					? simpleChar	\
Exp1: complex_char					? complexChar	\
Exp1: float						? expFloat	\
Exp1: 'nil'						? expNil	\
Exp1: String								\
Exp1: '(' '{' OptArgListComma '}' ')'			? expArray	\
Exp1: '(' '[' OptAssocListComma ']' ')'			? expMapping	\
Exp1: ident						? expVar	\
Exp1: '(' ListExp ')'					? parsed_1_	\
Exp1: FunctionCall '(' OptArgList ')'			? expFuncall	\
Exp1: 'catch' '(' ListExp ')'				? expCatch	\
Exp1: 'new' OptObject String				? expNew1	\
Exp1: 'new' OptObject '(' ListExp ')'			? expNew2	\
Exp1: 'new' OptObject String '(' OptArgList ')'		? expNew3	\
Exp1: 'new' OptObject '(' ListExp ')' '(' OptArgList ')'		\
							? expNew4	\
Exp1: Exp2 '->' ident '(' OptArgList ')'		? expCallOther	\
Exp1: Exp2 '<-' String					? expInstance1	\
Exp1: Exp2 '<-' '(' ListExp ')'				? expInstance2	" +
"\
Exp2: Exp1								\
Exp2: Exp2 '[' ListExp ']'				? expIndex	\
Exp2: Exp2 '[' ListExp '..' ListExp ']'			? expRange	" +
"\
PostfixExp: Exp2							\
PostfixExp: PostfixExp '++'				? expPostIncr	\
PostfixExp: PostfixExp '--'				? expPostDecr	" +
"\
PrefixExp: PostfixExp							\
PrefixExp: '++' CastExp					? expPreIncr	\
PrefixExp: '--' CastExp					? expPreDecr	\
PrefixExp: '+' CastExp					? expPlus	\
PrefixExp: '-' CastExp					? expMinus	\
PrefixExp: '!' CastExp					? expNot	\
PrefixExp: '~' CastExp					? expNegate	" +
"\
CastExp: PrefixExp							\
CastExp: '(' ClassType Stars ')' CastExp		? expCast	" +
"\
MultExp: CastExp							\
MultExp: MultExp '*' CastExp				? expMult	\
MultExp: MultExp '/' CastExp				? expDiv	\
MultExp: MultExp '%' CastExp				? expMod	" +
"\
AddExp: MultExp								\
AddExp: AddExp '+' MultExp				? expAdd	\
AddExp: AddExp '-' MultExp				? expSub	" +
"\
ShiftExp: AddExp							\
ShiftExp: ShiftExp '<<' AddExp				? expLShift	\
ShiftExp: ShiftExp '>>' AddExp				? expRShift	" +
"\
RelExp: ShiftExp							\
RelExp: RelExp '<' ShiftExp				? expLess	\
RelExp: RelExp '>' ShiftExp				? expGreater	\
RelExp: RelExp '<=' ShiftExp				? expLessEq	\
RelExp: RelExp '>=' ShiftExp				? expGreaterEq	" +
"\
EquExp: RelExp								\
EquExp: EquExp '==' RelExp				? expEqual	\
EquExp: EquExp '!=' RelExp				? expUnequal	" +
"\
BitandExp: EquExp							\
BitandExp: BitandExp '&' EquExp				? expAnd	" +
"\
BitxorExp: BitandExp							\
BitxorExp: BitxorExp '^' BitandExp			? expXor	" +
"\
BitorExp: BitxorExp							\
BitorExp: BitorExp '|' BitxorExp			? expOr		" +
"\
AndExp: BitorExp							\
AndExp: AndExp '&&' BitorExp				? expLand	" +
"\
OrExp: AndExp								\
OrExp: OrExp '||' AndExp				? expLor	" +
"\
CondExp: OrExp								\
CondExp: OrExp '?' ListExp ':' CondExp			? expQuest	" +
"\
Exp: CondExp								\
Exp: CondExp '=' Exp					? expAssign	\
Exp: CondExp '+=' Exp					? expAsgnAdd	\
Exp: CondExp '-=' Exp					? expAsgnSub	\
Exp: CondExp '*=' Exp					? expAsgnMult	\
Exp: CondExp '/=' Exp					? expAsgnDiv	\
Exp: CondExp '%=' Exp					? expAsgnMod	\
Exp: CondExp '<<=' Exp					? expAsgnLShift	\
Exp: CondExp '>>=' Exp					? expAsgnRShift	\
Exp: CondExp '&=' Exp					? expAsgnAnd	\
Exp: CondExp '^=' Exp					? expAsgnXor	\
Exp: CondExp '|=' Exp					? expAsgnOr	" +
"\
ListExp: Exp								\
ListExp: ListExp ',' Exp				? expComma	\
OptListExp:						? opt		\
OptListExp: ListExp							" +
"\
ArgList: Exp								\
ArgList: ArgList ',' Exp						\
OptArgList:						? noArguments	\
OptArgList: ArgList					? arguments	\
OptArgList: ArgList '...'				? ellipsis	\
OptArgListComma:					? list		\
OptArgListComma: ArgList				? noCommaList	\
OptArgListComma: ArgList ','				? noCommaList	" +
"\
AssocPair: Exp ':' Exp					? parsed_0_2_	\
AssocList: AssocPair							\
AssocList: AssocList ',' AssocPair					\
OptAssocListComma:					? list		\
OptAssocListComma: AssocList				? noCommaList	\
OptAssocListComma: AssocList ','			? noCommaList	",
			  source);
    return (parsed) ? parsed[0] : nil;
}

/*
 * NAME:	program()
 * DESCRIPTION: ({ ({ }), ({ }) })
 */
static mixed *program(mixed *parsed)
{
    return ({ new LPCProgram(parsed[0], parsed[1]) });
}

/*
 * NAME:	inh()
 * DESCRIPTION:	({ private, "inherit", label, LPCExpression })
 */
static mixed *inh(mixed *parsed)
{
    return ({ new LPCInherit(parsed[0], parsed[2], parsed[3]) });
}

/*
 * NAME:	classType()
 * DESCRIPTION:	({ "private", "int" })
 */
static mixed *classType(mixed *parsed)
{
    return ({ implode(parsed, " "), nil });
}

/*
 * NAME:	classTypeName()
 * DESCRIPTION:	({ "private", "object", LPCExpression })
 */
static mixed *classTypeName(mixed *parsed)
{
    int sz;

    sz = sizeof(parsed);
    return ({ implode(parsed[.. sz - 2], " "), parsed[sz - 1] });
}

/*
 * NAME:	noCommaList()
 * DESCRIPTION:	({ parsed1, ",", parsed2 })
 */
static mixed *noCommaList(mixed *parsed)
{
    return ({ parsed - ({ "," }) });
}

/*
 * NAME:	dataDecl()
 * DESCRIPTION:	({ "private int", nil, ({ ({ 0, "a" }), ({ 1, "b" }) }) })
 */
static mixed *dataDecl(mixed *parsed)
{
    mixed *decls;
    int i, sz;
    LPCType type;

    decls = parsed[2];
    for (i = 0, sz = sizeof(decls); i < sz; i++) {
	type = new LPCType(parsed[0], parsed[1], decls[i][0]);
	decls[i] = (sizeof(decls[i]) == 2) ?
		    new LPCVariable(type, decls[i][1]) :
		    new LPCFunction(type, decls[i][1], decls[i][3], decls[i][4],
				    nil);
    }
    return decls;
}

/*
 * NAME:	noArguments()
 * DESCRIPTION:	({ })
 */
static mixed *noArguments(mixed *parsed)
{
    return ({ ({ }), FALSE });
}

/*
 * NAME:	arguments()
 * DESCRIPTION:	({ parsed1, ",", parsed2 })
 */
static mixed *arguments(mixed *parsed)
{
    return ({ parsed - ({ "," }), FALSE });
}

/*
 * NAME:	ellipsis()
 * DESCRIPTION:	({ parsed1, ",", parsed2, "..." })
 */
static mixed *ellipsis(mixed *parsed)
{
    return ({ parsed[.. sizeof(parsed) - 2] - ({ "," }), TRUE });
}

/*
 * NAME:	formal()
 * DESCRIPTION:	({ "private int", nil, 0, "a" })
 */
static mixed *formal(mixed *parsed)
{
    return ({ new LPCDeclaration(new LPCType(parsed[0], parsed[1], parsed[2]),
				 parsed[3]) });
}

/*
 * NAME:	formalMixed()
 * DESCRIPTION:	({ "a" })
 */
static mixed *formalMixed(mixed *parsed)
{
    return ({ new LPCDeclaration(new LPCType("mixed", nil, 0), parsed[0]) });
}

/*
 * NAME:	functionDecl()
 * DESCRIPTION:	({ "private int", nil, 0, "func", ({ }), FALSE, LPCStmtBlock })
 */
static mixed *functionDecl(mixed *parsed)
{
    return ({ new LPCFunction(new LPCType(parsed[0], parsed[1], parsed[2]),
			      parsed[3], parsed[5], parsed[6], parsed[8]) });
}

/*
 * NAME:	voidDecl()
 * DESCRIPTION:	({ "private", nil, "func", ({ }), FALSE, LPCStmtBlock })
 */
static mixed *voidDecl(mixed *parsed)
{
    return ({ new LPCFunction(new LPCType(parsed[0], parsed[1], 0),
			   parsed[2], parsed[4], parsed[5], parsed[7]) });
}

/*
 * NAME:	listStmt()
 * DESCRIPTION:	({ firstStmt, lastStmt, additionalStmt })
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
 * NAME:	expStmt()
 * DESCRIPTION:	({ LPCExpression, ";" })
 */
static mixed *expStmt(mixed *parsed)
{
    return ({ new LPCStmtExp(parsed[0]) });
}

/*
 * NAME:	emptyStmt()
 * DESCRIPTION:	({ ";" })
 */
static mixed *emptyStmt(mixed *parsed)
{
    return ({ new LPCStatement });
}

/*
 * NAME:	compoundStmt()
 * DESCRIPTION:	({ "{" ({ }), firstStmt, lastStmt, "}" })
 */
static mixed *compoundStmt(mixed *parsed)
{
    return ({ new LPCStmtBlock(parsed[1],
			       ((sizeof(parsed) == 3) ? nil : parsed[2])) });
}

/*
 * NAME:	expIntDec()
 * DESCRIPTION:	({ "1234" })
 */
static mixed *expIntDec(mixed *parsed)
{
    return ({ new LPCExpression((int) parsed[0]) });
}

/*
 * NAME:	expIntOct()
 * DESCRIPTION:	({ "0123" })
 */
static mixed *expIntOct(mixed *parsed)
{
    return ({ new LPCExpression(stringParser->octalInt(parsed[0])) });
}

/*
 * NAME:	expIntHex()
 * DESCRIPTION:	({ "0x123" })
 */
static mixed *expIntHex(mixed *parsed)
{
    return ({ new LPCExpression(stringParser->hexadecimalInt(parsed[0])) });
}

/*
 * NAME:	expFloat()
 * DESCRIPTION:	({ "12.34" })
 */
static mixed *expFloat(mixed *parsed)
{
    return ({ new LPCExpression((float) parsed[0]) });
}

/*
 * NAME:	simpleString()
 * DESCRIPTION:	({ "\"abc\"" })
static mixed *simpleString(mixed *parsed)
{
    string str;

    str = parsed[0];
    str = str[1 .. strlen(str) - 2];
    return ({ new LPCExpression(str) });
}

/*
 * NAME:	complexString()
 * DESCRIPTION:	({ "\"ab\\nc\"" })
 */
static mixed *complexString(mixed *parsed)
{
    string str;

    str = parsed[0];
    str = str[1 .. strlen(str) - 2];
    return ({ new LPCExpression(stringParser->parse(str)) });
}

/*
 * NAME:	simpleChar()
 * DESCRIPTION:	({ 'a' })
 */
static mixed *simpleChar(mixed *parsed)
{
    return ({ new LPCExpression(parsed[0][1]) });
}

/*
 * NAME:	complexChar()
 * DESCRIPTION:	({ "'\\n'" })
 */
static mixed *complexChar(mixed *parsed)
{
    string str;

    str = parsed[0];
    str = str[1 .. strlen(str) - 2];
    return ({ new LPCExpression(stringParser->parse(str)[0]) });
}

/*
 * NAME:	expNil()
 * DESCRIPTION:	({ "nil" })
 */
static mixed *expNil(mixed *parsed)
{
    return ({ new LPCExpression(nil) });
}

/*
 * NAME:	expArray()
 * DESCRIPTION:	({ "(", "{", ({ }), "}", ")" })
 */
static mixed *expArray(mixed *parsed)
{
    return ({ new LPCExpArray(parsed[2]) });
}

/*
 * NAME:	expMapping()
 * DESCRIPTION:	({ "(", "[", ({ }), "]", ")" })
 */
static mixed *expMapping(mixed *parsed)
{
    return ({ new LPCExpMapping(parsed[2]) });
}

/*
 * NAME:	expVar()
 * DESCRIPTION:	({ "a" })
 */
static mixed *expVar(mixed *parsed)
{
    return ({ new LPCExpVar(parsed[0]) });
}

/*
 * NAME:	expFuncall()
 * DESCRIPTION:	({ "func", "(", ({ }), FALSE, ")" })
 */
static mixed *expFuncall(mixed *parsed)
{
    switch (sizeof(parsed)) {
    case 5:
	return ({ new LPCExpFuncall(parsed[0], parsed[2], parsed[3]) });

    case 6:
	return ({ new LPCExpInhFuncall(nil, parsed[1], parsed[3], parsed[4]) });

    case 7:
	return ({ new LPCExpInhFuncall(parsed[0], parsed[2], parsed[4],
				       parsed[5]) });
    }
}

/*
 * NAME:	expCatch()
 * DESCRIPTION:	({ "catch", "(", LPCExpression, ")" })
 */
static mixed *expCatch(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_CATCH, parsed[2]) });
}

/*
 * NAME:	expNew1()
 * DESCRIPTION:	({ "new", LPCExpression })
 */
static mixed *expNew1(mixed *parsed)
{
    return ({ new LPCExpFuncall("new_object", ({ parsed[1] }), FALSE) });
}

/*
 * NAME:	expNew2()
 * DESCRIPTION:	({ "new", "(", LPCExpression, ")" })
 */
static mixed *expNew2(mixed *parsed)
{
    return ({ new LPCExpFuncall("new_object", ({ parsed[2] }), FALSE) });
}

/*
 * NAME:	expNew3()
 * DESCRIPTION:	({ "new", LPCExpression, "(", ({ }), FALSE, ")" })
 */
static mixed *expNew3(mixed *parsed)
{
    return ({ new LPCExpFuncall("new_object", ({ parsed[1] }) + parsed[3],
			 parsed[4]) });
}

/*
 * NAME:	expNew4()
 * DESCRIPTION:	({ "new", "(", LPCExpression, ")", "(", ({ }), FALSE, ")" })
 */
static mixed *expNew4(mixed *parsed)
{
    return ({ new LPCExpFuncall("new_object", ({ parsed[2] }) + parsed[5],
			     parsed[6]) });
}

/*
 * NAME:	expCallOther()
 * DESCRIPTION:	({ LPCExpression, "->", "func", "(", ({ }), FALSE, ")" })
 */
static mixed *expCallOther(mixed *parsed)
{
    return ({ new LPCExpFuncall("call_other",
				({ parsed[0], new LPCExpression(parsed[2]) }) + 
			     parsed[4],
			     parsed[5]) });
}

/*
 * NAME:	expInstance1()
 * DESCRIPTION:	({ LPCExpression, "<-", LPCExpression })
 */
static mixed *expInstance1(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_INSTANCEOF, parsed[0], parsed[2]) });
}

/*
 * NAME:	expInstance2()
 * DESCRIPTION:	({ LPCExpression, "<-", "(", LPCExpression, ")", })
 */
static mixed *expInstance2(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_INSTANCEOF, parsed[0], parsed[3]) });
}

/*
 * NAME:	expIndex()
 * DESCRIPTION:	({ LPCExpression, "[", LPCExpression, "]" })
 */
static mixed *expIndex(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_INDEX, parsed[0], parsed[2]) });
}

/*
 * NAME:	expRange()
 * DESCRIPTION:	({ LPCExpression, "[", LPCExpression, "..", LPCExpression,
 *		   "]" })
 */
static mixed *expRange(mixed *parsed)
{
    return ({ new LPCExp3(LPC_EXP_RANGE, parsed[0], parsed[2], parsed[4]) });
}

/*
 * NAME:	expPostIncr()
 * DESCRIPTION:	({ LPCExpression, "++" })
 */
static mixed *expPostIncr(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_POST_INCR, parsed[0]) });
}

/*
 * NAME:	expPostDecr()
 * DESCRIPTION:	({ LPCExpression, "--" })
 */
static mixed *expPostDecr(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_POST_DECR, parsed[0]) });
}

/*
 * NAME:	expPreIncr()
 * DESCRIPTION:	({ "++", LPCExpression })
 */
static mixed *expPreIncr(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_PRE_INCR, parsed[1]) });
}

/*
 * NAME:	expPreDecl()
 * DESCRIPTION:	({ "--", LPCExpression })
 */
static mixed *expPreDecr(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_PRE_DECR, parsed[1]) });
}

/*
 * NAME:	expPlus()
 * DESCRIPTION:	({ "+", LPCExpression })
 */
static mixed *expPlus(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_UPLUS, parsed[1]) });
}

/*
 * NAME:	expMinus()
 * DESCRIPTION:	({ "-", LPCExpression })
 */
static mixed *expMinus(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_UMIN, parsed[1]) });
}

/*
 * NAME:	expNot()
 * DESCRIPTION:	({ "!", LPCExpression })
 */
static mixed *expNot(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_NOT, parsed[1]) });
}

/*
 * NAME:	expNegate()
 * DESCRIPTION:	({ "~", LPCExpression })
 */
static mixed *expNegate(mixed *parsed)
{
    return ({ new LPCExp1(LPC_EXP_NEG, parsed[1]) });
}

/*
 * NAME:	expCast()
 * DESCRIPTION:	({ "(", "private int", nil, 0, ")", LPCExpression })
 */
static mixed *expCast(mixed *parsed)
{
    return ({ new LPCExpCast(new LPCType(parsed[1], parsed[2], parsed[3]),
			     parsed[5]) });
}

/*
 * NAME:	expMult()
 * DESCRIPTION:	({ LPCExpression, "*", LPCExpression })
 */
static mixed *expMult(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_MULT, parsed[0], parsed[2]) });
}

/*
 * NAME:	expDiv()
 * DESCRIPTION:	({ LPCExpression, "/", LPCExpression })
 */
static mixed *expDiv(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_DIV, parsed[0], parsed[2]) });
}

/*
 * NAME:	expMod()
 * DESCRIPTION:	({ LPCExpression, "%", LPCExpression })
 */
static mixed *expMod(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_MOD, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAdd()
 * DESCRIPTION:	({ LPCExpression, "+", LPCExpression })
 */
static mixed *expAdd(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ADD, parsed[0], parsed[2]) });
}

/*
 * NAME:	expSub()
 * DESCRIPTION:	({ LPCExpression, "-", LPCExpression })
 */
static mixed *expSub(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_SUB, parsed[0], parsed[2]) });
}

/*
 * NAME:	expLShift()
 * DESCRIPTION:	({ LPCExpression, "<<", LPCExpression })
 */
static mixed *expLShift(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LSHIFT, parsed[0], parsed[2]) });
}

/*
 * NAME:	expRShift()
 * DESCRIPTION:	({ LPCExpression, ">>", LPCExpression })
 */
static mixed *expRShift(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_RSHIFT, parsed[0], parsed[2]) });
}

/*
 * NAME:	expLess()
 * DESCRIPTION:	({ LPCExpression, "<", LPCExpression })
 */
static mixed *expLess(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LT, parsed[0], parsed[2]) });
}

/*
 * NAME:	expGreater()
 * DESCRIPTION:	({ LPCExpression, ">", LPCExpression })
 */
static mixed *expGreater(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_GT, parsed[0], parsed[2]) });
}

/*
 * NAME:	expLessEq()
 * DESCRIPTION:	({ LPCExpression, "<=", LPCExpression })
 */
static mixed *expLessEq(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LE, parsed[0], parsed[2]) });
}

/*
 * NAME:	expGreaterEq()
 * DESCRIPTION:	({ LPCExpression, ">=", LPCExpression })
 */
static mixed *expGreaterEq(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_GE, parsed[0], parsed[2]) });
}

/*
 * NAME:	expEqual()
 * DESCRIPTION:	({ LPCExpression, "==", LPCExpression })
 */
static mixed *expEqual(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_EQ, parsed[0], parsed[2]) });
}

/*
 * NAME:	expUnequal()
 * DESCRIPTION:	({ LPCExpression, "!=", LPCExpression })
 */
static mixed *expUnequal(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_NE, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAnd()
 * DESCRIPTION:	({ LPCExpression, "&", LPCExpression })
 */
static mixed *expAnd(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_AND, parsed[0], parsed[2]) });
}

/*
 * NAME:	expXor()
 * DESCRIPTION:	({ LPCExpression, "^", LPCExpression })
 */
static mixed *expXor(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_XOR, parsed[0], parsed[2]) });
}

/*
 * NAME:	expOr()
 * DESCRIPTION:	({ LPCExpression, "|", LPCExpression })
 */
static mixed *expOr(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_OR, parsed[0], parsed[2]) });
}

/*
 * NAME:	expLand()
 * DESCRIPTION:	({ LPCExpression, "&&", LPCExpression })
 */
static mixed *expLand(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LAND, parsed[0], parsed[2]) });
}

/*
 * NAME:	expLor()
 * DESCRIPTION:	({ LPCExpression, "||", LPCExpression })
 */
static mixed *expLor(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_LOR, parsed[0], parsed[2]) });
}

/*
 * NAME:	expQuest()
 * DESCRIPTION:	({ LPCExpression, "?", LPCExpression, ":", LPCExpression })
 */
static mixed *expQuest(mixed *parsed)
{
    return ({ new LPCExp3(LPC_EXP_QUEST, parsed[0], parsed[2], parsed[4]) });
}

/*
 * NAME:	expAssign()
 * DESCRIPTION:	({ LPCExpression, "=", LPCExpression })
 */
static mixed *expAssign(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnAdd()
 * DESCRIPTION:	({ LPCExpression, "+=", LPCExpression })
 */
static mixed *expAsgnAdd(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_ADD, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnSub()
 * DESCRIPTION:	({ LPCExpression, "-=", LPCExpression })
 */
static mixed *expAsgnSub(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_SUB, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnMult()
 * DESCRIPTION:	({ LPCExpression, "*=", LPCExpression })
 */
static mixed *expAsgnMult(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_MULT, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnDiv()
 * DESCRIPTION:	({ LPCExpression, "/=", LPCExpression })
 */
static mixed *expAsgnDiv(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_DIV, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnMod()
 * DESCRIPTION:	({ LPCExpression, "%=", LPCExpression })
 */
static mixed *expAsgnMod(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_MOD, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnLShift()
 * DESCRIPTION:	({ LPCExpression, "<<=", LPCExpression })
 */
static mixed *expAsgnLShift(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_LSHIFT, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnRShift()
 * DESCRIPTION:	({ LPCExpression, ">>=", LPCExpression })
 */
static mixed *expAsgnRShift(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_RSHIFT, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnAnd()
 * DESCRIPTION:	({ LPCExpression, "&=", LPCExpression })
 */
static mixed *expAsgnAnd(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_AND, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnXor()
 * DESCRIPTION:	({ LPCExpression, "^=", LPCExpression })
 */
static mixed *expAsgnXor(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_XOR, parsed[0], parsed[2]) });
}

/*
 * NAME:	expAsgnOr()
 * DESCRIPTION:	({ LPCExpression, "|=", LPCExpression })
 */
static mixed *expAsgnOr(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_ASSIGN_OR, parsed[0], parsed[2]) });
}

/*
 * NAME:	expComma()
 * DESCRIPTION:	({ LPCExpression, ",", LPCExpression })
 */
static mixed *expComma(mixed *parsed)
{
    return ({ new LPCExp2(LPC_EXP_COMMA, parsed[0], parsed[2]) });
}

/*
 * NAME:	ifStmt()
 * DESCRIPTION:	({ "if", "(", LPCExpression, ")", LPCStatement, nil })
 */
static mixed *ifStmt(mixed *parsed)
{
    return ({ new LPCStmtCond(parsed[2], parsed[4], parsed[5]) });
}

/*
 * NAME:	doWhileStmt()
 * DESCRIPTION:	({ "do", LPCStatement, "while", "(", LPCExpression, ")", ";" })
 */
static mixed *doWhileStmt(mixed *parsed)
{
    return ({ new LPCStmtDoWhile(parsed[1], parsed[4]) });
}

/*
 * NAME:	whileStmt()
 * DESCRIPTION:	({ "while", "(", LPCExpression, ")", LPCStatement })
 */
static mixed *whileStmt(mixed *parsed)
{
    return ({ new LPCStmtFor(nil, parsed[2], nil, parsed[4]) });
}

/*
 * NAME:	forStmt()
 * DESCRIPTION:	({ "for", "(", LPCExpression, ";", LPCExpression, ";",
 *		   LPCExpression, LPCStatement })
 */
static mixed *forStmt(mixed *parsed)
{
    return ({ new LPCStmtFor(parsed[2], parsed[4], parsed[6], parsed[8]) });
}

/*
 * NAME:	rlimitsStmt()
 * DESCRIPTION:	({ "rlimits", "(", LPCExpression, ";", LPCExpression, ")",
 *		   LPCStatement })
 */
static mixed *rlimitsStmt(mixed *parsed)
{
    return ({ new LPCStmtRlimits(parsed[2], parsed[4], parsed[6]) });
}

/*
 * NAME:	catchErrStmt()
 * DESCRIPTION:	({ "catch", LPCBlockStmt, ":", LPCStatement })
 */
static mixed *catchErrStmt(mixed *parsed)
{
    return ({ new LPCStmtCatch(parsed[1], parsed[3]) });
}

/*
 * NAME:	catchStmt()
 * DESCRIPTION:	({ "catch", LPCBlockStmt })
 */
static mixed *catchStmt(mixed *parsed)
{
    return ({ new LPCStmtCatch(parsed[1], nil) });
}

/*
 * NAME:	switchStmt()
 * DESCRIPTION:	({ "switch", "(", LPCExpression, ")", LPCBlockStmt })
 */
static mixed *switchStmt(mixed *parsed)
{
    return ({ new LPCStmtSwitch(parsed[2], parsed[4]) });
}

/*
 * NAME:	caseStmt()
 * DESCRIPTION:	({ "case", LPCExpression, ":", LPCStatement })
 */
static mixed *caseStmt(mixed *parsed)
{
    return ({ new LPCStmtCase(parsed[1], parsed[3]) });
}

/*
 * NAME:	caseRangeStmt()
 * DESCRIPTION:	({ "case", LPCExpression, "..", LPCExpression, ":",
 *		   LPCStatement })
 */
static mixed *caseRangeStmt(mixed *parsed)
{
    return ({ new LPCStmtCaseRange(parsed[1], parsed[3], parsed[5]) });
}

/*
 * NAME:	defaultStmt()
 * DESCRIPTION:	({ "default", ":", LPCStatement })
 */
static mixed *defaultStmt(mixed *parsed)
{
    return ({ new LPCStmtDefault(parsed[2]) });
}

/*
 * NAME:	labelStmt()
 * DESCRIPTION:	({ "label", ":", LPCStatement })
 */
static mixed *labelStmt(mixed *parsed)
{
    return ({ new LPCStmtLabel(parsed[0], parsed[2]) });
}

/*
 * NAME:	gotoStmt()
 * DESCRIPTION:	({ "goto", "label", ";" })
 */
static mixed *gotoStmt(mixed *parsed)
{
    return ({ new LPCStmtGoto(parsed[1]) });
}

/*
 * NAME:	breakStmt()
 * DESCRIPTION:	({ "break", ";" })
 */
static mixed *breakStmt(mixed *parsed)
{
    return ({ new LPCStmtBreak });
}

/*
 * NAME:	continueStmt()
 * DESCRIPTION:	({ "continue", ";" })
 */
static mixed *continueStmt(mixed *parsed)
{
    return ({ new LPCStmtContinue });
}

/*
 * NAME:	returnExpStmt()
 * DESCRIPTION:	({ "return", LPCExpression, ";" })
 */
static mixed *returnExpStmt(mixed *parsed)
{
    return ({ new LPCStmtReturnExp(parsed[1]) });
}

/*
 * NAME:	returnStmt()
 * DESCRIPTION:	({ "return", ";" })
 */
static mixed *returnStmt(mixed *parsed)
{
    return ({ new LPCStmtReturn });
}
