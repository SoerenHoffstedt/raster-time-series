
#include <iostream>
#include <fstream>
#include <json/json.h>
#include "operators/print.h"
#include "operators/expression.h"
#include "operators/source/fake_source.h"


int main() {

    using namespace rts;

    std::ifstream file_in("../../test/query/test_query.json");

    Json::Value json_query;
    file_in >> json_query;

    FakeSource *s    = new FakeSource(json_query["fake_source_params"]);
    FakeSource *s2   = new FakeSource(json_query["fake_source_params"]);
    Expression *e    = new Expression(json_query["expression"], { s });
    Print *p         = new Print(json_query["print_params"], { e });
    p->consume();

    delete p;
    delete e;
    delete s;
    delete s2;

    return 0;
}
