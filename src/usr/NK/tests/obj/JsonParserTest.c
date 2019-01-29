#include <NKlib.h>

inherit Test;

private JsonParser jsonParser;

private void itShouldParseJson(void) {
    mapping json;

    json = jsonParser->parse("{\"string\": \"a string\", \"integer\": 42, \"float\": 42.42, \"boolTrue\": true, " +
                             "\"boolFalse\": false, \"null\": null}")["json"];

    expectEqual(TEST_LINE, "a string", json["string"]);
    expectEqual(TEST_LINE, 42, json["integer"]);
    expectEqual(TEST_LINE, 42.42, json["float"]);
    expectEqual(TEST_LINE, "true", json["boolTrue"]);
    expectEqual(TEST_LINE, "false", json["boolFalse"]);
    expectEqual(TEST_LINE, "null", json["null"]);
}

void runBeforeTests(void) {
    jsonParser = new JsonParser();
}

void runTests(void) {
    itShouldParseJson();
}
