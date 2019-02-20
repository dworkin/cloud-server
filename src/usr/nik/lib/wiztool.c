#include "nik.h"

inherit Terminal;
inherit NK_KFUN;

string getPrompt(object user) {
    return magenta(user->query_name() + "> ", TRUE);
}

void cmd_de(object user, string cmd, string str) {
    object experiment;
    int t, f, k;

    if (str) {
        if (!find_object(DICE_EXPERIMENT_OBJ)) {
            compile_object(DICE_EXPERIMENT_OBJ);
        }
        experiment = clone_object(DICE_EXPERIMENT_OBJ);
        if (str == "4k3") {
            experiment->run(user, DICE_EXPR_COUNT, 4, 6, 3);
            return;
        }
        if (sscanf(str, "%dd%dk%d", t, f, k) == 3) {
            experiment->run(user, DICE_EXPR_COUNT, t, f, k);
            return;
        }
        if (sscanf(str, "%dd%d", t, f) == 2) {
            experiment->run(user, DICE_EXPR_COUNT, t, f, t);
            return;
        }
    }
    user->println("Usage: de <experiment parameters>");
}

void cmd_dex(object user, string cmd, string str) {
    float target;
    int sampleSize;
    string filename;
    object experiment;

    if (!str || !sscanf(str, "%f in %s %d", target, filename, sampleSize) == 3) {
        user->println("Usage: dex 10 in ~/data/nik-data-1-3d6k3.dice 10000");
        return;
    }

    if (!find_object(DICE_EXPERIMENT_CHECKS_OBJ)) {
        compile_object(DICE_EXPERIMENT_CHECKS_OBJ);
    }
    experiment = clone_object(DICE_EXPERIMENT_CHECKS_OBJ);
    experiment->run(user, target, sampleSize, filename);
}
