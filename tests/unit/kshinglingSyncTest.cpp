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

void kshinglingSyncTest::setUp() {
    const int SEED = 617;
    srand(SEED);
}

void kshinglingSyncTest::tearDown() {}



void kshinglingSyncTest::testAll() {

    int string_len = 100;
    //randascii = shingles size log10()+1
    // CPISYNC k = 3 b = 38; k = 4 b = 46; k = 5 b = 54
    size_t shingle_len =7;//ceil(log(string_len));
    int editDistance_bar = 5;
    GenSync::SyncProtocol base_set_proto = GenSync::SyncProtocol::CPISync;
    //GenSync::SyncProtocol base_set_proto = GenSync::SyncProtocol::CPISync;
    char stopword = '$';

    string Alicetxt = randSampleTxt(string_len);
    string Bobtxt = randStringEdit(Alicetxt, editDistance_bar);



    GenSync Alice = GenSync::Builder().
            setProtocol(base_set_proto).
            setStringProto(GenSync::StringSyncProtocol::kshinglingSync).
            setComm(GenSync::SyncComm::socket).
            setShingleLen(shingle_len).
            setPort(8001).
            build();
    Alice.addStr(new DataObject(Alicetxt), false);


    GenSync Bob = GenSync::Builder().
            setProtocol(base_set_proto).
            setStringProto(GenSync::StringSyncProtocol::kshinglingSync).
            setComm(GenSync::SyncComm::socket).
            setShingleLen(shingle_len).
            setPort(8001).
            build();
    Bob.addStr(new DataObject(Bobtxt), true);

    forkHandleReport report = forkHandle(Alice, Bob, false);

//
//    multiset<string> alice_set;
//    for(auto item : Alice.dumpElements()) alice_set.insert(item->to_string());
//    multiset<string> bob_set;
//    for(auto item : Bob.dumpElements()) bob_set.insert(item->to_string());
//
//    CPPUNIT_ASSERT(multisetDiff(alice_set, bob_set).empty());// separate set recon success from string recon
    //CPISync Setup
    //kshinglingSync kshingling = kshinglingSync(base_set_proto, base_comm, 14+(shingle_len+2)*6,ceil(numDif*2.3), 0,0);

    //InteractiveCPISync Set up
    //kshinglingSync kshingling = kshinglingSync(base_set_proto, base_comm, 14+(shingle_len+2)*6, 7, 3, 0);

    //IBLTSync Setup
    //kshinglingSync kshingling = kshinglingSync(baseSetProto, base_comm, 8, 0, 0, numDif*10);

    string recoveredAlice = Alice.dumpString()->to_string();
if(recoveredAlice == Alicetxt)
    cout<<"i didnt change"<<endl;
//    cout << "Time: " + to_string(report.totalTime) << endl;
//    cout << "bitsTot: " + to_string(report.bytesTot) << endl;
//    cout << "bitsR: " + to_string(report.bytesRTot) << endl;
//    cout << "Btyes: "<< report.bytes<<endl;
//    if(recoveredAlice != Bobtxt) cout<< "enable stgring recon in GenSync"<<endl;
    CPPUNIT_ASSERT(recoveredAlice == Bobtxt);
//    CPPUNIT_ASSERT(report.success);

}

void kshinglingSyncTest::testIdividualCPI() {
    const int BITS = 8;
    const int EXP_NumE = 160;
    const int EXP_Diff = 40;


    GenSync Alice = GenSync::Builder().
            setProtocol(GenSync::SyncProtocol::CPISync).
            setComm(GenSync::SyncComm::socket).
            setMbar(1e3).
            build();


    GenSync Bob = GenSync::Builder().
            setProtocol(GenSync::SyncProtocol::CPISync).
            setComm(GenSync::SyncComm::socket).
            setMbar(1e3).
            build();
    clock_t t1 = clock();
    vector<ZZ> ALL_ELEM;
    for (int i = 0; i < EXP_NumE; ++i) {
        ALL_ELEM.push_back(StrtoZZ(randAsciiStr(4)));
    }

    for (int j = EXP_Diff; j < EXP_NumE; ++j) {
        Alice.addElem(new DataObject(ALL_ELEM[j]));
    }

    for (int j = 0; j < EXP_NumE; ++j) {
        Bob.addElem(new DataObject(ALL_ELEM[j]));
    }
    clock_t t2 = clock();

    forkHandleReport res = forkHandle(Alice,Bob, false);
    clock_t t3 = clock();

//    cout<<"Add Time: "<<to_string(double(t2-t1)/CLOCKS_PER_SEC)<<endl;
//    cout<<"Sync Time: "<<to_string(double(t3-t2)/CLOCKS_PER_SEC)<<endl;
//    cout << "Comm:" + to_string(res.bytes)<<endl;
//    cout << "Comm Tot:" + to_string(res.bytesTot)<<endl;
//    cout << "Time:" + to_string(res.totalTime)<<endl;

}