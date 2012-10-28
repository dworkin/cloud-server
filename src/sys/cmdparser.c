# include <type.h>
# include <cmdparser.h>

mapping verbs;		/* verb -> refcount */
mapping adverbs;	/* adverb -> 1 */
string *literals;	/* list of literals */
mapping quotes;		/* quote -> verb */
mapping synonyms;	/* synonym -> verb */
mapping verbsynonyms;	/* verb -> list of synonyms */
mapping prepositions;	/* preposition -> refcount */
mapping syntax;		/* verb;xyz -> 1 or list of prepositions */
string grammar;		/* grammar for commands */

void initialize();

/*
 * NAME:	create()
 * DESCRIPTION:	initialize command parser
 */
static void create()
{
    verbs = ([ ]);
    adverbs = ([ ]);
    literals = ({ });
    quotes = ([ ]);
    synonyms = ([ ]);
    verbsynonyms = ([ ]);
    prepositions = ([ ]);
    syntax = ([ ]);

    initialize();
}


static mixed *encaps(mixed *phrase)
{
    return ({ phrase });
}

static mixed *sentence(mixed *phrase)
{
    return phrase[.. 0] + ({ phrase[1 ..] });
}

static mixed *skip(mixed *phrase)
{
    return ({ });
}

static mixed *num(mixed *num)
{
    sscanf(num[0], "%d", num[0]); return num;
}

static mixed *num0(mixed *num)		{ num[0] = 0; return num; }
static mixed *num1(mixed *num)		{ num[0] = 1; return num; }
static mixed *num2(mixed *num)		{ num[0] = 2; return num; }
static mixed *num3(mixed *num)		{ num[0] = 3; return num; }
static mixed *num4(mixed *num)		{ num[0] = 4; return num; }
static mixed *num5(mixed *num)		{ num[0] = 5; return num; }
static mixed *num6(mixed *num)		{ num[0] = 6; return num; }
static mixed *num7(mixed *num)		{ num[0] = 7; return num; }
static mixed *num8(mixed *num)		{ num[0] = 8; return num; }
static mixed *num9(mixed *num)		{ num[0] = 9; return num; }
static mixed *num10(mixed *num)		{ num[0] = 10; return num; }
static mixed *num11(mixed *num)		{ num[0] = 11; return num; }
static mixed *num12(mixed *num)		{ num[0] = 12; return num; }
static mixed *num13(mixed *num)		{ num[0] = 13; return num; }
static mixed *num14(mixed *num)		{ num[0] = 14; return num; }
static mixed *num15(mixed *num)		{ num[0] = 15; return num; }
static mixed *num16(mixed *num)		{ num[0] = 16; return num; }
static mixed *num17(mixed *num)		{ num[0] = 17; return num; }
static mixed *num18(mixed *num)		{ num[0] = 18; return num; }
static mixed *num19(mixed *num)		{ num[0] = 19; return num; }
static mixed *num20(mixed *num)		{ num[0] = 20; return num; }
static mixed *num30(mixed *num)		{ num[0] = 30; return num; }
static mixed *num40(mixed *num)		{ num[0] = 40; return num; }
static mixed *num50(mixed *num)		{ num[0] = 50; return num; }
static mixed *num60(mixed *num)		{ num[0] = 60; return num; }
static mixed *num70(mixed *num)		{ num[0] = 70; return num; }
static mixed *num80(mixed *num)		{ num[0] = 80; return num; }
static mixed *num90(mixed *num)		{ num[0] = 90; return num; }
static mixed *num100(mixed *num)	{ num[0] = 100; return num; }
static mixed *num1000(mixed *num)	{ num[0] = 1000; return num; }
static mixed *num1e6(mixed *num)	{ num[0] = 1000000; return num; }
static mixed *num1e9(mixed *num)	{ num[0] = 1000000000; return num; }

static mixed *num100x(mixed *num)	{ return ({ num[0] * 100 }); }
static mixed *num1000x(mixed *num)	{ return ({ num[0] * 1000 }); }
static mixed *num1e6x(mixed *num)	{ return ({ num[0] * 1000000 }); }
static mixed *num1e9x(mixed *num)	{ return ({ num[0] * 1000000000 }); }

static mixed *num1_2(mixed *num)	{ return ({ num[0] + num[1] }); }
static mixed *num1_3(mixed *num)	{ return ({ num[0] + num[2] }); }

static string *concat(mixed *words)
{
    return ({ implode(words, "") });
}

static mixed *noadjectives(mixed *phrase)
{
    return ({ ({ }) });
}

static mixed *possessiveqs(mixed *poss)
{
    return ({ ({ poss[1][.. strlen(poss[1]) - 3] }) + poss[0] });
}

static mixed *possessivesq(mixed *poss)
{
    return ({ ({ poss[1][.. strlen(poss[1]) - 2] }) + poss[0] });
}

static mixed *none(mixed *phrase)
{
    return ({ nil });
}

static mixed *spec(mixed *spec)
{
    return ({ 0, 0, nil }) + spec;
}

static mixed *spec_a(mixed *spec)
{
    return ({ PARSER_SINGULAR | PARSER_SINGLE, 0, nil }) + spec[1 ..];
}

static mixed *spec_the(mixed *spec)
{
    return ({ PARSER_SPECIFIC, 0, nil }) + spec[1 ..];
}

static mixed *spec_poss(mixed *spec)
{
    return ({ 0, 0 }) + spec;
}

static mixed *spec_count(mixed *spec)
{
    return ({ PARSER_COUNT | ((spec[0] == 1) ?
			       PARSER_SINGULAR | PARSER_SINGLE :
			       PARSER_PLURAL | PARSER_MULTIPLE),
	      spec[0], nil, spec[1] });
}

static mixed *spec_count_poss(mixed *spec)
{
    return ({ PARSER_COUNT | PARSER_PLURAL | ((spec[0] == 1) ?
					       PARSER_SINGLE : PARSER_MULTIPLE),
	      spec[0] }) + spec[2 ..];
}

static mixed *spec_num(mixed *spec)
{
    return ({ PARSER_SINGULAR | PARSER_CARDINAL | PARSER_SINGLE,
	      spec[0], nil, spec[1] });
}

static mixed *spec_num_of(mixed *spec)
{
    return ({ PARSER_PLURAL | PARSER_CARDINAL | PARSER_SINGLE,
	      spec[0], nil, spec[2] });
}

static mixed *spec_num_poss(mixed *spec)
{
    return ({ PARSER_PLURAL | PARSER_CARDINAL | PARSER_SINGLE, spec[0] }) +
	   spec[2..];
}

static mixed *spec_all(mixed *spec)
{
    return ({ PARSER_PLURAL | PARSER_MULTIPLE, 0, nil }) + spec[1 ..];
}

static mixed *spec_all_poss(mixed *spec)
{
    return ({ PARSER_PLURAL | PARSER_MULTIPLE, 0 }) + spec[1 ..];
}

static mixed *ofphrase(mixed *phrase)
{
    return ({ ({ phrase[1 ..] }) });
}

static mixed *ofofphrase(mixed *phrase)
{
    return ({ phrase[0] + ({ phrase[2 ..] }) });
}

static mixed *nounphrase(mixed *phrase)
{
    return phrase + ({ nil });
}

static mixed *titlephrase(mixed *phrase)
{
    phrase[0] |= PARSER_TITLE;
    phrase[6] |= PARSER_SPECIFIC;
    return phrase[0 .. 4] + ({ phrase[6 ..] });
}

static mixed *obj_ref(mixed *phrase)
{
    return ({ 0, 0, nil, ({ }), phrase[0], nil });
}

static mixed *obj_count(mixed *phrase)
{
    return ({ PARSER_COUNT, phrase[0], nil, ({ }), phrase[2], nil });
}

static mixed *obj_numeric(mixed *phrase)
{
    return ({ PARSER_CARDINAL, phrase[0], nil, ({ }), phrase[2], nil });
}

static mixed *obj_all(mixed *phrase)
{
    return ({ 0, 0, nil, ({ }), "all", nil });
}

static mixed *obj_all_ref(mixed *phrase)
{
    return ({ PARSER_MULTIPLE, 0, nil, ({ }), phrase[2], nil });
}

static mixed *quote(mixed *phrase)
{
    string str;
    int i;

    str = phrase[0];
    for (i = 1; str[i] == ' ' || str[i] == '\t'; i++) ;
    return ({ ({ 0, quotes[str[i .. i]], str[i + 1 ..] }) });
}

static mixed *literal(mixed *phrase)
{
    string verb, str;
    int i;

    str = phrase[1];
    for (i = sizeof(literals); --i >= 0; ) {
	verb = literals[i];
	if (sscanf(str, verb + "%s", str) != 0) {
	    i = (strlen(str) != 0 && (str[0] == ' ' || str[0] == '\t'));
	    return ({ phrase[0], verb, str[i ..] });
	}
    }
}

static mixed *prep(mixed *prep)
{
    return (sizeof(prep) != 1) ? ({ implode(prep, " ") }) : prep;
}

static mixed *adverb(mixed *phrase)
{
    return (adverbs[phrase[0]]) ? phrase : nil;
}

static mixed *adverb2(mixed *phrase)
{
    return ({ phrase[1], phrase[0] }) + phrase[2 ..];
}

static mixed *adverb3(mixed *phrase)
{
    return ({ phrase[2] }) + phrase[0 .. 1] + phrase[3 ..];
}

static mixed adverblast(mixed *phrase)
{
    int len;

    len = sizeof(phrase);
    return ({ phrase[len - 1] }) + phrase[.. len - 2];
}

/*
 * NAME:	make_grammar()
 * DESCRIPTION:	construct a grammar for the current set of verbs and
 *		prepositions
 */
private void make_grammar()
{
    /*
     * base grammar
     */
    grammar = "\
Sentence: VerbPhrase			? encaps \
Sentence: Sentence And VerbPhrase	? sentence \
"+"\
And: ','				? skip \
And: 'and'				? skip \
And: ',' 'and'				? skip \
"+"\
OptAnd: \
OptAnd: And \
"+"\
Separator: And \
Separator: ';'				? skip \
Separator: '.'				? skip \
"+"\
OptComma: \
OptComma: ','				? skip \
"+"\
N1_9: 'one'				? num1 \
N1_9: 'two'				? num2 \
N1_9: 'three'				? num3 \
N1_9: 'four'				? num4 \
N1_9: 'five'				? num5 \
N1_9: 'six'				? num6 \
N1_9: 'seven'				? num7 \
N1_9: 'eight'				? num8 \
N1_9: 'nine'				? num9 \
"+"\
N20_90: 'twenty'			? num20 \
N20_90: 'thirty'			? num30 \
N20_90: 'forty'				? num40 \
N20_90: 'fifty'				? num50 \
N20_90: 'sixty'				? num60 \
N20_90: 'seventy'			? num70 \
N20_90: 'eighty'			? num80 \
N20_90: 'ninety'			? num90 \
"+"\
Nnn: N1_9 \
Nnn: 'eleven'				? num11 \
Nnn: 'twelve'				? num12 \
Nnn: 'thirteen'				? num13 \
Nnn: 'fourteen'				? num14 \
Nnn: 'fifteen'				? num15 \
Nnn: 'sixteen'				? num16 \
Nnn: 'seventeen'			? num17 \
Nnn: 'eighteen'				? num18 \
Nnn: 'nineteen'				? num19 \
Nnn: N20_90 '-' N1_9			? num1_3 \
"+"\
N1_99: Nnn \
N1_99: 'ten'				? num10 \
N1_99: N20_90 \
"+"\
N100: 'hundred'				? num100 \
N100: Nnn 'hundred'			? num100x \
"+"\
Nx100: N1_99 \
Nx100: N100 \
Nx100: N100 OptAnd N1_99		? num1_2 \
"+"\
Nx: N1_99 \
Nx: N1_9 'hundred'			? num100x \
"+"\
N1000: 'thousand'			? num1000 \
N1000: Nx 'thousand'			? num1000x \
"+"\
Nx1000: Nx100 \
Nx1000: N1000 \
Nx1000: N1000 OptAnd Nx100		? num1_2 \
"+"\
N1e6: 'million'				? num1e6 \
N1e6: Nx 'million'			? num1e6x \
"+"\
Nx1e6: Nx1000 \
Nx1e6: N1e6 \
Nx1e6: N1e6 OptAnd Nx1000		? num1_2 \
"+"\
N1e9: 'billion'				? num1e9 \
N1e9: Nx 'billion'			? num1e9x \
"+"\
Nx1e9: Nx1e6 \
Nx1e9: N1e9 \
Nx1e9: N1e9 OptAnd Nx1e6		? num1_2 \
"+"\
C1_9: 'first'				? num1 \
C1_9: 'second'				? num2 \
C1_9: 'third'				? num3 \
C1_9: 'fourth'				? num4 \
C1_9: 'fifth'				? num5 \
C1_9: 'sixth'				? num6 \
C1_9: 'seventh'				? num7 \
C1_9: 'eighth'				? num8 \
C1_9: 'ninth'				? num9 \
"+"\
C20_90: 'twentieth'			? num20 \
C20_90: 'thirtieth'			? num30 \
C20_90: 'fortieth'			? num40 \
C20_90: 'fiftieth'			? num50 \
C20_90: 'sixtieth'			? num60 \
C20_90: 'seventieth'			? num70 \
C20_90: 'eightieth'			? num80 \
C20_90: 'ninetieth'			? num90 \
"+"\
C1_99: C1_9 \
C1_99: 'tenth'				? num10 \
C1_99: 'eleventh'			? num11 \
C1_99: 'twelfth'			? num12 \
C1_99: 'thirteenth'			? num13 \
C1_99: 'fourteenth'			? num14 \
C1_99: 'fifteenth'			? num15 \
C1_99: 'sixteenth'			? num16 \
C1_99: 'seventeenth'			? num17 \
C1_99: 'eighteenth'			? num18 \
C1_99: 'nineteenth'			? num19 \
C1_99: C20_90 \
C1_99: N20_90 '-' C1_9			? num1_3 \
"+"\
C100: 'hundredth'			? num100 \
C100: Nnn 'hundredth'			? num100x \
"+"\
Cx100: C1_99 \
Cx100: C100 \
Cx100: N100 OptAnd C1_99		? num1_2 \
"+"\
C1000: 'thousandth'			? num1000 \
C1000: Nx 'thousandth'			? num1000x \
"+"\
Cx1000: Cx100 \
Cx1000: C1000 \
Cx1000: N1000 OptAnd Cx100		? num1_2 \
"+"\
C1e6: 'millionth'			? num1e6 \
C1e6: Nx 'millionth'			? num1e6x \
"+"\
Cx1e6: Cx1000 \
Cx1e6: C1e6 \
Cx1e6: N1e6 OptAnd Cx1000		? num1_2 \
"+"\
C1e9: 'billionth'			? num1e9 \
C1e9: Nx 'billionth'			? num1e9x \
"+"\
Cx1e9: Cx1e6 \
Cx1e9: C1e9 \
Cx1e9: N1e9 OptAnd Cx1e6		? num1_2 \
"+"\
Count: number				? num \
Count: 'zero'				? num0 \
Count: Nx1e9 \
"+"\
Cardinal: stndrdth			? num \
Cardinal: Cx1e9 \
"+"\
Word: word \
Word: Word '-' word			? concat \
Word: number '-' word			? concat \
Word: number '-' number			? concat \
Word: Word '-' number			? concat \
"+"\
WordQS: wordqs \
WordQS: Word '-' wordqs			? concat \
"+"\
WordSQ: wordsq \
WordSQ: Word '-' wordsq			? concat \
"+"\
Adjs: Word \
Adjs: Adjs Word \
Adjectives: Adjs			? encaps \
"+"\
OptAdjs:				? noadjectives \
OptAdjs: Adjectives \
"+"\
Possessive: 'my' \
Possessive: 'your' \
Possessive: 'his' \
Possessive: 'her' \
Possessive: 'its' \
Possessive: 'our' \
Possessive: 'their' \
Possessive: OptThe OptAdjs WordQS	? possessiveqs \
Possessive: OptThe OptAdjs WordSQ	? possessivesq \
"+"\
OptThe: \
OptThe: 'the'				? skip \
"+"\
OptOf: \
OptOf: 'of'				? skip \
"+"\
OptPoss: 'the'				? none \
OptPoss: Possessive \
"+"\
SpecPhrase: OptAdjs			? spec \
SpecPhrase: 'a' OptAdjs			? spec_a \
SpecPhrase: 'an' OptAdjs		? spec_a \
SpecPhrase: 'the' OptAdjs		? spec_the \
SpecPhrase: Possessive OptAdjs		? spec_poss \
SpecPhrase: OptThe Count OptAdjs	? spec_count \
SpecPhrase: Count 'of' OptPoss OptAdjs	? spec_count_poss \
SpecPhrase: OptThe Cardinal OptAdjs	? spec_num \
SpecPhrase: OptThe Cardinal 'of' OptAdjs \
					? spec_num_of \
SpecPhrase: OptThe Cardinal 'of' OptPoss OptAdjs \
					? spec_num_poss \
SpecPhrase: 'all' OptAdjs		? spec_all \
SpecPhrase: 'all' OptOf OptPoss OptAdjs	? spec_all_poss \
"+"\
OfPhrase: 'of' SpecPhrase Word		? ofphrase \
OfPhrase: OfPhrase And 'of' SpecPhrase Word \
					? ofofphrase \
"+"\
NounPhrase: SpecPhrase Word		? nounphrase \
NounPhrase: SpecPhrase Word OfPhrase \
"+"\
TitlePhrase: NounPhrase \
TitlePhrase: SpecPhrase Word 'the' NounPhrase \
					? titlephrase \
"+"\
Ref: 'his' \
Ref: 'hers' \
Ref: 'its' \
Ref: 'theirs' \
Ref: 'them' \
"+"\
ObjPhrase: TitlePhrase \
ObjPhrase: Ref				? obj_ref \
ObjPhrase: 'one'			? obj_ref \
ObjPhrase: 'him'			? obj_ref \
ObjPhrase: 'her'			? obj_ref \
ObjPhrase: 'it'				? obj_ref \
ObjPhrase: 'them' 'all'			? obj_ref \
ObjPhrase: Count 'of' Ref		? obj_count \
ObjPhrase: OptThe Cardinal 'of' Ref	? obj_numeric \
ObjPhrase: 'all'			? obj_all \
ObjPhrase: 'all' 'of' Ref		? obj_all_ref \
"+"\
Except: 'except'			? skip \
Except: 'except' 'for'			? skip \
"+"\
ExceptPhrase: OptComma Except ObjPhrase OptComma \
					? encaps \
"+"\
ExceptObjPhrase: ObjPhrase		? encaps \
ExceptObjPhrase: ObjPhrase ExceptPhrase	? encaps \
"+"\
ObjsPhrase: ExceptObjPhrase \
ObjsPhrase: ObjsPhrase And ExceptObjPhrase \
"+"\
ObjectPhrase: ObjsPhrase		? encaps \
"+"\
Adverb: Word				? adverb \
"+"\
OptAdverb:				? none \
OptAdverb: Word				? adverb \
"+"\
VerbPhrase: Verb Adverb			? adverblast \
VerbPhrase: OptAdverb Verb \
VerbPhrase: Verb ObjectPhrase Adverb	? adverblast \
VerbPhrase: OptAdverb Verb ObjectPhrase \
whitespace = /[ \t\n]+/ ";

    /*
     * optional parts
     */
    if (map_sizeof(verbs) != 0) {
	grammar += "Verb:'" +
		   implode(map_indices(verbs) + map_indices(synonyms),
			   "'Verb:'") + "'";
    }
    if (map_sizeof(quotes) != 0) {
	grammar += "Sentence: quote ? quote quote=/\n[ \t]*" +
		   implode(map_indices(quotes), ".*/quote=/\n[ \t]*") +
		   ".*/";
    }
    if (sizeof(literals) != 0) {
	grammar += "VerbPhrase: OptAdverb literal ? literal literal=/" +
		   implode(literals, "([ \t,.;-].*)?/literal=/") +
		   "([ \t,.;-].*)?/";
    }
    if (map_sizeof(prepositions) != 0) {
	string preps;

	grammar += "\
VerbPhrase: Verb Adverb Prep ObjectPhrase		? adverb2 \
VerbPhrase: Verb Prep ObjectPhrase Adverb		? adverblast \
VerbPhrase: OptAdverb Verb Prep ObjectPhrase \
VerbPhrase: Verb ObjectPhrase Adverb Prep ObjectPhrase	? adverb3 \
VerbPhrase: Verb ObjectPhrase Prep ObjectPhrase Adverb	? adverblast \
VerbPhrase: OptAdverb Verb ObjectPhrase Prep ObjectPhrase ";
	preps = "Prep:'" +
		implode(map_indices(prepositions), "'?prep\tPrep:'") + "'?prep";
	grammar += implode(explode(preps, " "), "''") + " ";
    }

    /*
     * tokens
     */
    grammar += "\
number = /[0-9]+/ \
stndrdth = /(1st|2nd|3rd|[04-9]th)/ \
stndrdth = /[0-9]*([02-9](1st|2nd|3rd|04-9]th)|1[123]th)/ \
wordqs = /[^ \n\t,.;-]+'s/ \
wordsq = /[^ \n\t,.;-]+s'/ \
word = /[^ \n\t,.;-]+/";
}


/*
 * NAME:	add_rule()
 * DESCRIPTION:	add a rule to the grammar
 */
int add_rule(string verb, varargs int obj1, string prep, int obj2)
{
    if (verb && !synonyms[verb]) {
	string rule;

	rule = verb + (string) !!obj1 + ((prep) ? prep : "") + (string) !!obj2;
	if (syntax[rule]) {
	    return 0;	/* rule already exists */
	} else {
	    if (verbs[verb]) {
		verbs[verb]++;
	    } else {
		verbs[verb] = 1;
	    }
	    syntax[rule] = 1;
	}

	if (prep) {
	    if (prepositions[prep]) {
		prepositions[prep]++;
	    } else {
		prepositions[prep] = 1;
	    }
	}
	make_grammar();

	return 1;
    }

    return 0;
}

/*
 * NAME:	remove_rule()
 * DESCRIPTION:	remove a rule from the grammar
 */
int remove_rule(string verb, varargs int obj1, string prep, int obj2)
{
    string rule, *list;
    int i;

    rule = verb + (string) !!obj1 + ((prep) ? prep : "") + (string) !!obj2;
    if (syntax[rule]) {
	syntax[rule] = nil;
	if (--verbs[verb] == 0) {
	    /* remove synonyms too */
	    list = verbsynonyms[verb];
	    if (list) {
		for (i = sizeof(list); --i >= 0; ) {
		    synonyms[list[i]] = nil;
		}
		verbsynonyms[verb] = nil;
	    }
	    verbs[verb] = nil;
	}

	if (prep && --prepositions[prep] == 0) {
	    prepositions[prep] = nil;
	}
	make_grammar();
	return 1;
    }

    return 0;
}

/*
 * NAME:	add_synonym()
 * DESCRIPTION:	add a verb synonym
 */
int add_synonym(string synonym, string verb)
{
    if (synonym && verbs[verb] && !synonyms[synonym]) {
	synonyms[synonym] = verb;
	if (verbsynonyms[verb]) {
	    verbsynonyms[verb] += ({ synonym });
	} else {
	    verbsynonyms[verb] = ({ synonym });
	}

	make_grammar();
	return 1;
    }

    return 0;
}

/*
 * NAME:	remove_synonym()
 * DESCRIPTION:	remove a verb synonym
 */
int remove_synonym(string synonym)
{
    string verb, *list;

    if (synonym) {
	verb = synonyms[synonym];
	if (verb) {
	    synonyms[synonym] = nil;
	    list = verbsynonyms[verb] - ({ synonym });
	    verbsynonyms[verb] = (sizeof(list) != 0) ? list : nil;

	    make_grammar();
	    return 1;
	}
    }

    return 0;
}

/*
 * NAME:	add_adverb()
 * DESCRIPTION:	add an adverb
 */
void add_adverb(string adverb)
{
    if (adverb) {
	adverbs[adverb] = 1;
    }
}

/*
 * NAME:	remove_adverb()
 * DESCRIPTION:	remove an adverb
 */
void remove_adverb(string adverb)
{
    adverbs[adverb] = nil;
}

/*
 * NAME:	add_literal()
 * DESCRIPTION:	add a literal
 */
void add_literal(string literal)
{
    if (literal) {
	literals |= ({ literal });
	make_grammar();
    }
}

/*
 * NAME:	remove_literal()
 * DESCRIPTION:	remove a literal
 */
void remove_literal(string literal)
{
    literals -= ({ literal });
    make_grammar();
}

/*
 * NAME:	add_quote()
 * DESCRIPTION:	add a quote
 */
void add_quote(string quote, string verb)
{
    if (quote && verb) {
	quotes[quote] = verb;
	make_grammar();
    }
}

/*
 * NAME:	remove_quote()
 * DESCRIPTION:	remove a quote
 */
void remove_quote(string quote)
{
    quotes[quote] = nil;
    make_grammar();
}

/*
 * NAME:	parse()
 * DESCRIPTION:	parse a command
 */
mixed **parse(string str)
{
    mixed **commands, *command;
    int i, j, size;
    string verb, rule;

    commands = parse_string(grammar, "\n" + str);
    if (commands) {
	for (i = sizeof(commands); --i >= 0; ) {
	    command = commands[i];
	    size = sizeof(command);
	    if (size != 3 || typeof(command[2]) != T_STRING) {
		verb = command[1];
		str = synonyms[verb];
		if (str) {
		    command[1] = verb = str;
		}

		rule = verb;
		j = 2;
		if (j != size && typeof(command[j]) == T_ARRAY) {
		    rule += "1";
		    j++;
		} else {
		    rule += "0";
		}
		if (j != size && typeof(command[j]) == T_STRING) {
		    rule += command[j++];
		}
		if (j != size && typeof(command[j]) == T_ARRAY) {
		    rule += "1";
		} else {
		    rule += "0";
		}

		if (!syntax[rule]) {
		    return nil;
		}
	    }
	}
    }

    return commands;
}

/*
 * NAME:	initialize()
 * DESCRIPTION:	initialize parser
 */
void initialize()
{
    add_literal("say");
    add_literal("emote");
    add_quote("'", "say");
    add_quote(":", "emote");

    add_rule("go", 1);
    add_rule("look");
    add_rule("inventory");
    add_rule("get", 1);
    add_rule("examine", 1);
    add_rule("drop", 1);
    add_rule("pull", 1);
}

/* debug */
mixed query_verbs() { return verbs; }
mixed query_adverbs() { return adverbs; }
mixed query_literals() { return literals; }
mixed query_quotes() { return quotes; }
mixed query_synonyms() { return synonyms; }
mixed query_verbsynonyms() { return verbsynonyms; }
mixed query_prepositions() { return prepositions; }
mixed query_syntax() { return syntax; }
mixed query_grammar() { return grammar; }
