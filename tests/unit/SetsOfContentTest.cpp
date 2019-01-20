//
// Created by Bowen Song on 12/9/18.
//
#include "SetsOfContentTest.h"
CPPUNIT_TEST_SUITE_REGISTRATION(SetsOfContentTest);

void SetsOfContentTest::SelfUnitTest() {
//
    shingle_hash Shingle_A{.first = 298273671273648, .second = 198273671273645, .occurr = 1990, .lvl = 3,
            .compose = cycle{.cyc = 1, .head = 123, .len = 333}};
//    shingle_hash Shingle_B{.first = {298273671273645, 198273671273645}, .occurr = 1990, .second = 1231243798798123};
//
//    shingle_hash Shingle_C{.first = {198273671273645, 198273671273645}, .occurr = 1990, .second = 1231243798798123};
////
//    auto a1 = ZZtoShingleHash(ShingleHashtoZZ(Shingle_A));
//    if (a1 == Shingle_A)
//        cout<<"we are done"<<endl;

//    vector<size_t> tmp{298273671273648};
//    auto t = recursion(tmp);
    CPPUNIT_ASSERT(Shingle_A == ZZtoShingleHash(ShingleHashtoZZ(Shingle_A)));

//    CPPUNIT_ASSERT(Shingle_A != Shingle_B);
//    CPPUNIT_ASSERT(Shingle_C < Shingle_A);

//
////    cout << Shingle_A << endl;
//    cycle cyc_A{.head = 298273671273645, .tail = 1231239, .cyc = 9128};
//    cycle cyc_B{.head = 298273671273645, .tail = 1231239, .cyc = 938};
//    CPPUNIT_ASSERT(cyc_A == cyc_A);
//    CPPUNIT_ASSERT(cyc_A != cyc_B);
//    cout<<"Size of a Shingle_hash: "<<sizeof(shingle_hash)<<endl;
//    cout<<"Size of a cycle element: " << sizeof(cycle)<<endl;
}


void SetsOfContentTest::testAll() {
    string alicetxt = randSampleTxt(2000000); // 20MB is top on MAC

    DataObject *atxt = new DataObject(alicetxt);

    GenSync Alice = GenSync::Builder().
            setStringProto(GenSync::StringSyncProtocol::SetsOfContent).
            setProtocol(GenSync::SyncProtocol::IBLTSyncSetDiff).
            setComm(GenSync::SyncComm::socket).
            setTerminalStrSize(100).
            setNumPartitions(10).
            setlvl(4).
            setPort(8003).
            build();


//    string bobtxt = randStringEdit(alicetxt, 10);
    string bobtxt = randStringEditBurst(alicetxt, 100);

    DataObject *btxt = new DataObject(bobtxt);

    GenSync Bob = GenSync::Builder().
            setStringProto(GenSync::StringSyncProtocol::SetsOfContent).
            setProtocol(GenSync::SyncProtocol::IBLTSyncSetDiff).
            setComm(GenSync::SyncComm::socket).
            setTerminalStrSize(100).
            setNumPartitions(10).
            setlvl(4).
            setPort(8003).
            build();

    Bob.addStr(btxt, false);


    auto str_s = clock();

    Alice.addStr(atxt, false);
    double str_time = (double) (clock() - str_s) / CLOCKS_PER_SEC;


    auto recon_t = clock();
    auto report = forkHandle(Alice, Bob, false);
    double recon_time = (double) (clock() - recon_t) / CLOCKS_PER_SEC;


    string finally = Alice.dumpString()->to_string();

    cout << "CPU Time: " + to_string(report.CPUtime) << endl;
    cout << "Time: " + to_string(report.totalTime) << endl;
    cout << "bitsTot: " + to_string(report.bytesTot) << endl;
    cout << "bitsR: " + to_string(report.bytesRTot) << endl;
    cout << "Btyes: " << report.bytes << endl;
    cout << "String Add Time: "<< str_time<<endl;
    cout << "Rest of the Recon time: " <<recon_time<<endl;
    delete btxt;
    delete atxt;
    CPPUNIT_ASSERT(finally == bobtxt);

}