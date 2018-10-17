//
// Created by Bowen on 9/24/18.
//
#include <climits>
#include "kshinglingSyncTest.h"
//#include "GenSync.h"
//#include "kshinglingSync.h"
//#include "TestAuxiliary.h"
#include "kshingling.h"  // rest should not be here
#include "CommSocket.h"
#include "InterCPISync.h"
#include "TestAuxiliary.h"

CPPUNIT_TEST_SUITE_REGISTRATION(kshinglingSyncTest);

kshinglingSyncTest::kshinglingSyncTest() {}
kshinglingSyncTest::~kshinglingSyncTest() {}

void kshinglingSyncTest::setUp() {}

void kshinglingSyncTest::tearDown() {}

void kshinglingSyncTest::testAll() {

    int string_len = 500;

    // CPISYNC k = 3 b = 38; k = 4 b = 46; k = 5 b = 54
    int shingle_len = 4;
    int editDistance_bar = 73;
    GenSync::SyncProtocol base_set_proto = GenSync::SyncProtocol::InteractiveCPISync;
    GenSync::SyncComm base_comm = GenSync::SyncComm::socket;

    string Alicetxt = randAsciiStr(string_len);
    K_Shingle Alice_content = K_Shingle(shingle_len);
    string Bobtxt = randStringEdit(Alicetxt, editDistance_bar);
    K_Shingle Bob_content = K_Shingle(shingle_len);

    //see the actual num of diff
    auto Alice_set = Alice_content.getShingleSet_str(Alicetxt);
    auto Bob_set = Bob_content.getShingleSet_str(Bobtxt);
    int numDif = multisetDiff(Alice_set, Bob_set).size();

//    CPPUNIT_ASSERT(editDistance_bar * (shingleLen - 1) + 4 >= numDif);

    //number of difference between should alwasy be editDistance_bar*(shingleLen-1)

    //CPISync Setup
    //kshinglingSync kshingling = kshinglingSync(base_set_proto, base_comm, 14+(shingle_len+2)*8,ceil(numDif*2.3), 0,0);

    //InteractiveCPISync Set up
    kshinglingSync kshingling = kshinglingSync(base_set_proto, base_comm, 14+(shingle_len+2)*6, 7, 3, 0);

    //IBLTSync Setup
    //kshinglingSync kshingling = kshinglingSync(baseSetProto, base_comm, 8, 0, 0, numDif*10);


    GenSync Alice = kshingling.SyncHost(Alicetxt, Alice_content);

    GenSync Bob = kshingling.SyncHost(Bobtxt, Bob_content);

    forkHandleReport report = kshingling.SyncNreport(Alice, Bob);



    CPPUNIT_ASSERT(report.success);
    cout << "numDif: " + to_string(numDif) << endl;
    cout << "bits: " + to_string(report.bytes) << endl;
    cout << "bitsTot: " + to_string(report.bytesTot) << endl;
    cout << "bitsR: " + to_string(report.bytesRTot) << endl;
    //CPPUNIT_ASSERT(report.bytesTot<string_len*8+14);
    auto resa = kshingling.getString(Alice, Alice_content);
    CPPUNIT_ASSERT(resa == Bob_content.getOriginString());
    //CPPUNIT_ASSERT((resa == resb)==(resa!="" && resb!=""));  //either succeed or fail


//    syncTest(GenSyncServer, GenSyncClient);

}