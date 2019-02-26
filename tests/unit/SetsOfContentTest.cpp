//
// Created by Bowen Song on 12/9/18.
//
#include "SetsOfContentTest.h"
CPPUNIT_TEST_SUITE_REGISTRATION(SetsOfContentTest);

void SetsOfContentTest::SelfUnitTest() {
//

    shingle_hash Shingle_A{.first = 298273671273648, .second = 198273671273645, .occurr = 1990, .lvl = 3};
//    shingle_hash Shingle_B{.first = {298273671273645, 198273671273645}, .occurr = 1990, .second = 1231243798798123};
//
//    shingle_hash Shingle_C{.first = {198273671273645, 198273671273645}, .occurr = 1990, .second = 1231243798798123};
////
//    auto a1 = ZZtoShingleHash(ShingleHashtoZZ(Shingle_A));
//    if (a1 == Shingle_A)
//        cout<<"we are done"<<endl;

//    vector<size_t> tmp{298273671273648};
//    auto t = recursion(tmp);
    CPPUNIT_ASSERT(Shingle_A == ZZtoShingleHash(TtoZZ(Shingle_A)));

//
//
//    Resources initRes;
//    initResources(initRes);
//    auto a=randSampleTxt(2e6);
//    resourceMonitor(initRes,500,5e9);
//    //    resourceReport(initRes);
//    resourceReport(initRes);
//    cout<<initRes.VmemUsed<<endl;
////    CPPUNIT_ASSERT(Shingle_A != Shingle_B);
////    CPPUNIT_ASSERT(Shingle_C < Shingle_A);
//
//    auto recon_t = clock();
//    string tmp = randCharacters(2e5);
//    cout<<"String Creation time: "<<(double) (clock() - recon_t) / CLOCKS_PER_SEC<<endl;
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

    Resources initRes;
//    initResources(initRes);

    string alicetxt = randSampleTxt(2e6); // 20MB is top on MAC
    int partition =4;
    int lvl =4;
    int space = 4;
    int shingleLen = 2;



    DataObject *atxt = new DataObject(alicetxt);

    GenSync Alice = GenSync::Builder().
            setStringProto(GenSync::StringSyncProtocol::SetsOfContent).
            setProtocol(GenSync::SyncProtocol::CPISync).
            setComm(GenSync::SyncComm::socket).
            setTerminalStrSize(10).
            setNumPartitions(partition).
            setShingleLen(shingleLen).
            setSpace(space).
            setlvl(lvl).
            setPort(8001).
            build();


//    string bobtxt = randStringEdit(alicetxt, 10);
//    string bobtxt = randStringEdit((*atxt).to_string(),2e3);

    string bobtxt = randStringEditBurst(alicetxt, 1e3,"./tests/SampleTxt.txt");
    if(bobtxt.size()<pow(partition,lvl))
        bobtxt += randCharacters(pow(partition,lvl)-bobtxt.size());

    DataObject *btxt = new DataObject(bobtxt);

    GenSync Bob = GenSync::Builder().
            setStringProto(GenSync::StringSyncProtocol::SetsOfContent).
            setProtocol(GenSync::SyncProtocol::CPISync).
            setComm(GenSync::SyncComm::socket).
            setTerminalStrSize(10).
            setNumPartitions(partition).
            setShingleLen(shingleLen).
            setSpace(space).
            setlvl(lvl).
            setPort(8001).
            build();

    Bob.addStr(btxt, false);


    auto str_s = clock();

    Alice.addStr(atxt, false);
    double str_time = (double) (clock() - str_s) / CLOCKS_PER_SEC;


    auto recon_t = clock();
    auto report = forkHandle(Alice, Bob, false);
    double recon_time = (double) (clock() - recon_t) / CLOCKS_PER_SEC;
//    HeapProfilerDump("BO");
    resourceReport(initRes);
    string finally = Alice.dumpString()->to_string();



    writeStrToFile("Alice.txt",alicetxt);
    writeStrToFile("Bob.txt",bobtxt);

    auto r_res = getRsyncStats("Alice.txt","Bob.txt",true);
    cout<<"rsync comm cost: "<<r_res.recv+r_res.xmit<<endl;
    cout<<"Set of Content cost: "<<to_string(report.bytesRTot+report.bytesXTot)<<endl;

    cout << "CPU Time: " + to_string(report.CPUtime) << endl;
    cout << "Time: " + to_string(report.totalTime) << endl;
    cout << "bitsTot: " + to_string(report.bytesXTot) << endl;
    cout << "bitsR: " + to_string(report.bytesRTot) << endl;


    cout << "Terminal Str Trans: ------------------ " << Alice.getCustomResult("Literal comm")<<endl;
    cout << "Set Comm: ------------------ " << report.bytesXTot+report.bytesRTot-Alice.getCustomResult("Terminal comm")<<endl;
    cout << "Number of node diff: " << Alice.getCustomResult("Partition Sym Diff") << endl;
    cout << "String Reconstruction Time: " << Alice.getCustomResult("Str Reconstruction Time")<<endl;
    cout << "String Add Time: "<< str_time<<endl;
    cout << "Rest of the Recon time: " <<recon_time<<endl;
    delete btxt;
    delete atxt;
    CPPUNIT_ASSERT(finally == bobtxt);
    CPPUNIT_ASSERT(report.success);

}