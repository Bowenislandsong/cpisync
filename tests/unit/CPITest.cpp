//
// Created by Bowen Song on 10/30/18.
//

#include "CPITest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CPITest);

void CPITest::CPISyncTest() {
    long bits = sizeof(randAsciiStr())*8;
    long Diff = 20;
    long Tot = 20000;

    vector <ZZ> S_A, S_B;


    for (int i = 0; i < Tot; ++i) {
        ZZ tmp = TtoZZ(randAsciiStr());
        S_A.push_back(tmp);
        S_B.push_back(tmp);
    }
    for (int j = 0; j < Diff; ++j) {
        S_A.push_back(TtoZZ(randAsciiStr()));
        S_B.push_back(TtoZZ(randAsciiStr()));
    }

    GenSync Alice = GenSync::Builder().
            setProtocol(GenSync::SyncProtocol::CPISync).
            setComm(GenSync::SyncComm::socket).
            setBits(bits).
            setErr(64).
            setMbar(100).
            setPort(8001).
            build();
    for (auto e : S_A) {
        Alice.addElem(new DataObject(e));
    }




    GenSync Bob = GenSync::Builder().
            setProtocol(GenSync::SyncProtocol::CPISync).
            setComm(GenSync::SyncComm::socket).
            setBits(bits).
            setErr(64).
            setMbar(100).
            setPort(8001).
            build();


    for (auto e : S_B) {
        Bob.addElem(new DataObject(e));
    }

    forkHandleReport res = forkHandle(Alice,Bob,false);

    cout<<"CPI Comm Cost:"<<res.bytesRTot + res.bytesXTot<<endl;



    GenSync Alice_I = GenSync::Builder().
            setProtocol(GenSync::SyncProtocol::InteractiveCPISync).
            setComm(GenSync::SyncComm::socket).
            setBits(bits).
            setErr(8).
            setNumPartitions(3).
            setMbar(5).
            setPort(8001).
            build();

    for (auto e : S_A) {
        Alice_I.addElem(new DataObject(e));
    }



    GenSync Bob_I = GenSync::Builder().
            setProtocol(GenSync::SyncProtocol::InteractiveCPISync).
            setComm(GenSync::SyncComm::socket).
            setBits(bits).
            setErr(8).
            setNumPartitions(3).
            setMbar(5).
            setPort(8001).
            build();

    for (auto e : S_B) {
        Bob_I.addElem(new DataObject(e));
    }

    res = forkHandle(Alice_I,Bob_I, false);

    cout<<"InterCPI Comm Cost:"<<res.bytesRTot + res.bytesXTot<<endl;
    cout<<"InterCPI Time Cost:"<<res.totalTime<<endl;


}

