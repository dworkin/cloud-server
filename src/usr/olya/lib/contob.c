private int id;
private int type;

int callMe(object hasMessage) {
    switch (type) {
        case 1:
            acos(2.);
        case 2:
            hasMessage->message("caught math error in " + id + ": " + catch(acos(2.)) + "\n");
        case 3:
            tls_set("x::y", "z");
        case 4:
            hasMessage->message("caught bad arg in " + id + ": " + catch(tls_set("x::y", "z")) + "\n");
    }
    hasMessage->message("callMe from " + id + "\n");

    return id;
}

static void create(int id, int type) {
    ::id = id;
    ::type = type;
}
