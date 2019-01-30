#include <NKlib.h>

inherit "~nik/lib/wiztool";

static String startIntegration(void) {
    return new String("\n" + bold("Probabilities:") + "\n");
}

static String integrationStep(String str, int f) {
    str += ralign("P(" + f + ")", 5) + " = " + new SimpsonsRuleIntegrator(new ExponentialPDF((float) f))->integrate(0., 15.) + "\n";

    return str;
}

static int displayIntegrationResults(String str, object user) {
    user->message(str->buffer()->chunk());

    return 1;
}

void cmd_integrate(object user, string cmd, string arg) {
    Continuation continuation;
    Iterator iterator;
    int from, to;

    if (!arg || sscanf(arg, "%d %d", from, to) != 2) {
        user->message("Using default values 1..20\n");
        from = 1;
        to = 20;
    }

    if (from < 1) {
        user->message("The value for 'from' should be non-zero, positive.");
    }

    continuation = new Continuation("startIntegration");
    iterator = new IntIterator(from, to);
    while (!iterator->end()) {
	continuation >>= new ChainedContinuation("integrationStep", iterator->next());
    }
    continuation >>= new ChainedContinuation("displayIntegrationResults", user);
    continuation += new Continuation("done", user);

    continuation->runNext();
}

void cmd_dcont(object user, string cmd, string arg) {
    Continuation c;
    object obj1, obj2, obj3, obj4, obj5;

    if (!find_object("~olya/obj/contob")) {
        compile_object("~olya/obj/contob");
    }

    obj1 = clone_object("~olya/obj/contob", 1, 1);
    obj2 = clone_object("~olya/obj/contob", 2, 2);
    obj3 = clone_object("~olya/obj/contob", 3, 3);
    obj4 = clone_object("~olya/obj/contob", 4, 4);
    obj5 = clone_object("~olya/obj/contob", 5, 5);

    c = new DistContinuation(({
            obj1, obj2, obj3, obj4, obj5
        }), 0.001, "callMe", user);
    c += new Continuation("experimentCompleted", user);
    c->runNext();
}

void cmd_ccont(object user, string cmd, string arg) {
    Continuation c;

    c = new Continuation("c1", user);
    c >>= new ChainedContinuation("c2", user);
    c >>= new ChainedContinuation("c3", user);
    c >>= new ChainedContinuation("c4", user);
    c >>= new ChainedContinuation("c5", user);
    c += new Continuation("experimentCompleted", user);
    c->runNext();
}

void cmd_icont(object user, string cmd, string arg) {
    Continuation c;

    c = new IterativeContinuation(new IntIterator(1, 5), "icont", user);
    c += new Continuation("experimentCompleted", user);
    c->runNext();
}

static string c1(object hasMessage) {
    return "1";
}

static string c2(string x, object hasMessage) {
    hasMessage->message("caught math error: " + catch(acos(2.)) + "\n");
    return x + "2";
}

static string c3(string x, object hasMessage) {
    hasMessage->message("caught bad arg: " + catch(tls_set("x::y", "z")) + "\n");
    return x + "3";
}

static string c4(string x, object hasMessage) {
    return x + "4";
}

static void c5(string x, object hasMessage) {
    hasMessage->message("x = " + x + "\n");
}

static void icont(int id, object hasMessage) {
    switch (id) {
        case 2:
            hasMessage->message("icont caught math error in " + id + ": " + catch(acos(2.)) + "\n");
        case 4:
            hasMessage->message("icont caught bad arg in " + id + ": " + catch(tls_set("x::y", "z")) + "\n");
    }
    hasMessage->message("icont called with " + id + "\n");
}

static void experimentCompleted(object user) {
    user->message("\nDone.\n");
}
