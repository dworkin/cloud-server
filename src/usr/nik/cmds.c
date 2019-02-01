#include <NKlib.h>

inherit Terminal;
inherit NK_KFUN;

static void println(object user, string str) {
    user->message(str + "\n");
}

static void done(object user) {
    println(user, "Done.");
}

/* broadcast command */
void userCommandBroadcast(object user) {
    Continuation continuation;
    object *u;

    u = users() - ({ user });
    continuation = new DistContinuation(u, 0.1, "message", "Your significant mind notices the fabric of space.\n");
    println(user, "Your broadcast creates ripples along the fabric of space.");

    continuation->runNext();
}
/* broadcast command */

/* cls command */
void userCommandClearScreen(object user) {
    user->message(new Terminal()->clear());
}
/* cls command */

/* colors command */
void userCommandColors(object user) {
    Terminal t;
    Iterator iterator;
    int *heatMap;

    heatMap = allocate_int(7);
    heatMap[0] = 46;
    heatMap[1] = 226;
    heatMap[2] = 220;
    heatMap[3] = 214;
    heatMap[4] = 208;
    heatMap[5] = 202;
    heatMap[6] = 196;
    iterator = new IntIterator(0, 6);
    t = new Terminal();

    while (!iterator->end()) {
        user->message(t->color256(heatMap[iterator->next()], "AaBbCc  "));
    }

    println(user, "Done.");
}
/* colors command */

/* de command */
void userCommandDiceExperiment(object user, varargs string str) {
    object experiment;
    int t, f, k;

    user->message("\nRunning experiment...\n");
    experiment = clone_object(DICE_EXPERIMENT_OBJ);
    if (str && str == "4k3") {
        experiment->run(user, DICE_EXPR_COUNT, 4, 6, 3);
    } else if (str && sscanf(str, "%dd%dk%d", t, f, k) == 3) {
        experiment->run(user, DICE_EXPR_COUNT, t, f, k);
    } else if (str && sscanf(str, "%dd%d", t, f) == 2) {
        experiment->run(user, DICE_EXPR_COUNT, t, f, t);
    } else {
        user->message("Unknown parameters for experiment.\n");
    }
}
/* de command */

/* dex command */
static string getFilename(string filename) {
    return NK_DATA_DIR + "/" + filename;
}

static mixed *visitTheData(string filename, float target) {
    Statistics stats;
    DataVisitor visitor;

    stats = new Statistics(({ }));
    stats->restore(filename);

    visitor = new DataVisitor(target);
    stats->each(visitor);

    return ({ stats, visitor });
}

static Array getFrequency(mixed *objs) {
    mapping results;
    Statistics stats;
    DataVisitor visitor;

    ({ stats, visitor }) = objs;

    results = visitor->getVisitResults();
    return new Array(({
        new Rational(results["<"], stats->size())->toFloat() * 100. + "%",
        new Rational(results["="], stats->size())->toFloat() * 100. + "%",
        new Rational(results[">"], stats->size())->toFloat() * 100. + "%"
    }));
}

static int showFrequencyResults(Array frequency, object user) {
    println(user, "\nVisit results:\n" + frequency->reduce(new ArrayToMarkedListReducer(), 0, 2, 1));
}

void userCommandDiceExperimentChecks(object user, string str) {
    float target;
    string filename;
    Continuation continuation;

    if (!str || !sscanf(str, "%f in %s", target, filename) == 2) {
        println(user, "dex 10 in olya-data-1-3d6k3.dice");
        return;
    }

    continuation = new Continuation("getFilename", filename);
    continuation >>= new ChainedContinuation("visitTheData", target);
    continuation >>= new ChainedContinuation("getFrequency");
    continuation >>= new ChainedContinuation("showFrequencyResults", user);
    continuation += new Continuation("done", user);

    continuation->runNext();
}
/* dex command */

/* dice command */
static Polynomial *buildRollProbabilities(void) {
    Polynomial p1, p2, p3, p4, p5, p6;

    p1 = new Polynomial(({ 0., 1., 1., 1., 1., 1., 1. }));
    p2 = new Polynomial(({ 0., 0., 1., 1., 1., 1., 1. }));
    p3 = new Polynomial(({ 0., 0., 0., 1., 1., 1., 1. }));
    p4 = new Polynomial(({ 0., 0., 0., 0., 1., 1., 1. }));
    p5 = new Polynomial(({ 0., 0., 0., 0., 0., 1., 1. }));
    p6 = new Polynomial(({ 0., 0., 0., 0., 0., 0., 1. }));

    return ({ p1, p2, p3, p4, p5, p6 });
}

static Polynomial *rollProbabilitiesMathsRoundOne(Polynomial *p) {
    return ({
        p[0] * p[0] * p[0] * p[0],
        p[1] * p[1] * p[1] * p[1],
        p[2] * p[2] * p[2] * p[2],
        p[3] * p[3] * p[3] * p[3],
        p[4] * p[4] * p[4] * p[4],
        p[5] * p[5] * p[5] * p[5],
    });
}

static Polynomial *rollProbabilitiesMathsRoundTwo(Polynomial *p) {
    return ({
        (p[0] - p[1])[1..],
        (p[1] - p[2])[2..],
        (p[2] - p[3])[3..],
        (p[3] - p[4])[4..],
        (p[4] - p[5])[5..],
        p[5][6..]
    });
}

static Polynomial computeRollProbabilitiesPolynomial(Polynomial *p) {
    return p[0] + p[1] + p[2] + p[3] + p[4] + p[5];
}

static Polynomial displayRollProbabilitiesResults(Polynomial p, object user) {
    string str;

    str = "\n";
    str += "Four-keep-three (4k3) probabilities:\n " +
           p->toString("x") +
           "\nThus, the probability of rolling a 10 is the coefficient of x raised to the " +
           "10th power, which is 122. Because we have rolled four die, we divide that by " +
           "six raised to the 4th power.\n";

    println(user, break_string(str, 78));

    return p;
}

static int displayRollProbabilitiesExample(Polynomial p, object user, varargs int target, int n, int f) {
    Function reducer;
    mapping probs;
    Rational l, e, g;
    Array array;

    if (!target) {
        target = 10;
    }

    if (!n) {
        n = 4;
    }

    if (!f) {
        f = 6;
    }

    reducer = new ProbabilityReducer(p, f, n);
    reducer->apply(target);
    probs = reducer->evaluate();

    l = new Rational(probs["<"]);
    e = new Rational(probs["="]);
    g = new Rational(probs[">"]);

    array = new Array(({
        "roll < " + target + ": " + l->toString() + " = " + (probs["<"] * 100.) + "%",
        "roll = " + target + ": " + e->toString() + " = " + (probs["="] * 100.) + "%",
        "roll > " + target + ": " + g->toString() + " = " + (probs[">"] * 100.) + "%"
    }));

    println(user, "Rolling probabilities:\n" + array->reduce(new ArrayToListReducer(), 0, 2, 1));

    return 0;
}

static void rollDice(object user) {
    Array array;
    Function toListReducer, arithmeticReducer;
    DiceRoller roller;

    roller = new DiceRoller(4, 6, 3);
    array = new Array(roller->roll());
    toListReducer = new ArrayToListReducer();
    arithmeticReducer = new ArrayArithmeticReducer(ARRAY_ARITHMETIC_ADD);

    println(user, "Rolling 4K3 yields:\n" +
        array->reduce(toListReducer, 0, 2, 1) +
        " = " + array->reduce(arithmeticReducer) + "\n");
}

private Continuation fourKeepThreeContinuation(object user) {
    Continuation continuation;

    continuation = new Continuation("buildRollProbabilities");
    continuation >>= new ChainedContinuation("rollProbabilitiesMathsRoundOne");
    continuation >>= new ChainedContinuation("rollProbabilitiesMathsRoundTwo");
    continuation >>= new ChainedContinuation("computeRollProbabilitiesPolynomial");
    continuation >>= new ChainedContinuation("displayRollProbabilitiesResults", user);
    continuation >>= new ChainedContinuation("displayRollProbabilitiesExample", user);
    continuation += new Continuation("rollDice", user);

    return continuation;
}

static Polynomial getProbabilitiesForRoll(int n, int f) {
    return new DiceProbability(n, f);
}

private Continuation probabilitiesOfRoll(object user, int target, int n, int f) {
    Continuation continuation;

    continuation = new Continuation("getProbabilitiesForRoll", n, f);
    continuation >>= new ChainedContinuation("displayRollProbabilitiesExample", user, target, n, f);

    return continuation;
}

void userCommandDice(object user, string str) {
    Continuation continuation;
    int t, n, f;

    if (!str) {
        println(user, "dice <# on #d#|4k3>");
        return;
    }

    if (str == "4k3") {
        continuation = fourKeepThreeContinuation(user);
    } else if (sscanf(str, "%d on %dd%d", t, n, f) == 3) {
        continuation = probabilitiesOfRoll(user, t, n, f);
    } else {
        println(user, "dice <# on #d#|4k3>");
        return;
    }

    continuation += new Continuation("done", user);

    continuation->runNext();
}
/* dice command */

/* dontPanic command */
static void dontPanic(object user) {
    println(user, "DON'T PANIC");
}

static void largeFriendlyLetters(object user) {
    println(user, "(note the large, friendly letters)");
}

void userCommandDontPanic(object user) {
    Continuation continuation;

    continuation = new Continuation("dontPanic", user);
    continuation += new DelayedContinuation(0.5);
    continuation += new Continuation("largeFriendlyLetters", user);

    continuation->runNext();
}
/* dontPanic command */

/* integrate command */
static String startIntegration(void) {
    return new String("Probabilities:\n");
}

static String integrationStep(String str, int f) {
    str += ralign("P(" + f + ")", 5) + " = "
           + new SimpsonsRuleIntegrator(new ExponentialPDF((float) f))->integrate(0., 15.) + "\n";

    return str;
}

static int displayIntegrationResults(String str, object user) {
    user->message(str->buffer()->chunk());

    return 1;
}

void userCommandIntegrate(object user) {
    Continuation continuation;
    Iterator iterator;

    continuation = new Continuation("startIntegration");
    iterator = new IntIterator(1, 20);
    while (!iterator->end()) {
        continuation >>= new ChainedContinuation("integrationStep", iterator->next());
    }
    continuation >>= new ChainedContinuation("displayIntegrationResults", user);
    continuation += new Continuation("done", user);

    continuation->runNext();
}
/* integrate command */

/* json command */
void userCommandJson(object user) {
    JsonParser jsonParser;
    string payload;
    mapping json;

    jsonParser = new JsonParser();

    json = jsonParser->parse(read_file(NK_ETC_DIR + "/r2d2.json"))["json"];
    println(user, lalign("Name:", 8) + json["name"]);
    println(user, lalign("Height:", 8) + json["height"]);
    println(user, lalign("Mass:", 8) + json["mass"]);
    println(user, (string) sizeof(json["films"]) + " films");
}
/* json command */
