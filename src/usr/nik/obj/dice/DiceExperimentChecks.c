#include "nik.h"

static string getFilename(string filename) {
    return NK_DATA_DIR + "/" + filename;
}

static mixed *visitTheData(string filename, float target, int sampleSize) {
    Statistics stats;
    DataVisitor visitor;

    if (!find_object(DATA_GENERATOR)) {
        compile_object(DATA_GENERATOR);
    }
    stats = clone_object(DATA_GENERATOR, sampleSize);
    stats->restore(filename);

    visitor = new DataVisitor(target);
    stats->each(visitor);

    return ({ stats, visitor });
}

static Array getFrequency(mixed *objs) {
    mapping results;
    Statistics stats;
    DataVisitor visitor;
    Array array;

    ({ stats, visitor }) = objs;

    results = visitor->getVisitResults();
    array = new Array(({
        new Rational(results["<"], stats->size())->toFloat() * 100. + "%",
        new Rational(results["="], stats->size())->toFloat() * 100. + "%",
        new Rational(results[">"], stats->size())->toFloat() * 100. + "%"
    }));
    destruct_object(stats);

    return array;
}

static int showFrequencyResults(Array frequency, object user) {
    user->println("\nVisit results:\n" + frequency->reduce(new ArrayToMarkedListReducer(), 0, 2, 1));
}

static void diceExperimentChecksDone(object user) {
    user->println("Done.");
    user->showPrompt();
}

static void startChecking(object user) {
    user->println("\nChecking the results...");
}

void run(object user, float target, int sampleSize, string filename) {
    Continuation continuation;

    continuation = new Continuation("startChecking", user);
    continuation += new Continuation("getFilename", filename);
    continuation >>= new ChainedContinuation("visitTheData", target, sampleSize);
    continuation >>= new ChainedContinuation("getFrequency");
    continuation >>= new ChainedContinuation("showFrequencyResults", user);
    continuation += new Continuation("diceExperimentChecksDone", user);

    continuation->runNext();
}
