#include <NKlib.h>

inherit Terminal;
inherit NK_KFUN;

static void step0(object user) {
    user->message("\nGetting second data set for comparison...\n");
}

static void step1(DiceDataGenerator generator, int times, int faces, int keep) {
    generator->setStartTime();
    generator->rollTheDice(times, faces, keep);
}

static void step2(DiceDataGenerator generator) {
    generator->findTheSumAndMean();
}

static void step3(DiceDataGenerator generator) {
    generator->sort();
}

static void step4(DiceDataGenerator generator) {
    generator->findTheMinAndMaxAndMedian();
}

static void step5(DiceDataGenerator generator) {
    generator->findTheVarianceAndStandardDeviation();
}

static void step5a(DiceDataGenerator generator, string fileName) {
    generator->setStopTime();
    generator->save(fileName);
}

static void step6(DiceDataGenerator generator, object user) {
    user->message(generator->toString());
}

private void relationships(DiceDataGenerator *generators, object user, int showSortedRelationship) {
    DiceDataGenerator g1, g2;
    Array array;
    string *str;
    string label;
    int pad;
    float covariance, correlation;

    pad = 13;
    ({ g1, g2 }) = generators;
    label = "Relationships";
    if (showSortedRelationship) {
        g1->sort();
        g2->sort();
        label = "Sorted relationships";
    }
    ({ covariance, correlation }) = g1->covarianceAndCorrelationWith(g2);
    str = allocate(2);
    str[0] = bold(lalign("Covariance:", pad)) + " " + covariance;
    str[1] = bold(lalign("Correlation:", pad)) + " " + correlation;

    user->message("\n" + label + ":\n" + (new Array(str))->reduce(new ArrayToListReducer(), 0, 1, 1));
}

static void step7(DiceDataGenerator *generators, object user) {
    relationships(generators, user, FALSE);
}

static void step8(DiceDataGenerator *generators, object user) {
    relationships(generators, user, TRUE);
}

static void reportDone(string *fileNames, object user) {
    Array array;

    array = new Array(fileNames);
    user->message("\nThe data sets have been saved:\n");
    user->message(array->reduce(new ArrayToListReducer(), 0, sizeof(fileNames) - 1, 1));
    user->message("\nDone.\n");
    destruct_object(this_object());
}

private string buildFileName(object user, int count, int times, int faces, int keep) {
    return NK_DATA_DIR + "/" + user->query_name() + "-data-" +
        count + "-" + times + "d" + faces + "k" + keep + ".dice";
}

void run(object user, int sampleSize, int times, int faces, int keep) {
    Continuation continuation;
    Statistics g1, g2;
    string f1, f2;

    f1 = buildFileName(user, 1, times, faces, keep);
    f2 = buildFileName(user, 2, times, faces, keep);

    g1 = new DiceDataGenerator(sampleSize);
    g2 = new DiceDataGenerator(sampleSize);

    continuation = new Continuation("step1", g1, times, faces, keep);
    continuation += new Continuation("step2", g1);
    continuation += new Continuation("step3", g1);
    continuation += new Continuation("step4", g1);
    continuation += new Continuation("step5", g1);
    continuation += new Continuation("step5a", g1, f1);
    continuation += new Continuation("step6", g1, user);
    continuation += new Continuation("step0", user);
    continuation += new Continuation("step1", g2, times, faces, keep);
    continuation += new Continuation("step2", g2);
    continuation += new Continuation("step3", g2);
    continuation += new Continuation("step4", g2);
    continuation += new Continuation("step5", g2);
    continuation += new Continuation("step5a", g2, f2);
    continuation += new Continuation("step6", g2, user);
    continuation += new Continuation("step7", ({ g1, g2 }), user);
    continuation += new Continuation("step8", ({ g1, g2 }), user);
    continuation += new Continuation("reportDone", ({ f1, f2 }), user);

    continuation->runNext();
}
