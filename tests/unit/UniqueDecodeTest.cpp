//
// Created by Bowen Song on 10/7/18.
//

#include "UniqueDecodeTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(UniqueDecodeTest);

UniqueDecodeTest::UniqueDecodeTest() {}
UniqueDecodeTest::~UniqueDecodeTest() {}

void UniqueDecodeTest::UDTest() {
    string txt = "katana";
    UniqueDecode host = UniqueDecode(2);
    host.injectStr(txt);

    //host.reconstructDFS(host.getShingleSet(txt));
}