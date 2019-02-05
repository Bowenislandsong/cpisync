//
// Created by Bowen Song on 9/27/18.
//
#include "kshinglingSyncPerfTest.h"

// K-Shingling method does not scale because as string size increase,
// the lenght of each shingle needs to increase inorder to compensate the expoential time of baktracking
// In addition, as shingle size increase, the size for each element increases thus increase the communicaotn cost.
// At last, as shingle size increaases, the amount of differencecs between two shinlge set increases, due to shingle's redundancy
// set size increase as string sizxe increase.

CPPUNIT_TEST_SUITE_REGISTRATION(KshingleSyncPerf);
KshingleSyncPerf::KshingleSyncPerf() = default;
KshingleSyncPerf::~KshingleSyncPerf() = default;

// DEFAULT SETTINGS
const int shingleLen = 4;
const int editDist = 20;
const int strSize = 5;

const int tesPts = 6;// Test Pts per graph
const int target_confidence = 1;// Confidence interval
const int confidenceCap = 40; // after edit distance exceed confidenceCap, confidence go to 1.





void KshingleSyncPerf::kshingleTest3D(){
    PerformanceData test = PerformanceData(tesPts);
    vector<int> editDistRange;
//   vector<int> strSizeRange = {400, 600, 800, 1000, 1400, 1800};//, 2000, 2200, 2600, 3000};//, 5000, 7000, 9000, 10000};
//
//    test.kshingle3D(GenSync::SyncProtocol::CPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randSampleTxt);


    vector<int> strSizeRange = {400, 600, 800, 1000, 1400, 1800};//, 5000, 7000, 9000, 10000};
    test.kshingle3D(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, target_confidence, randAsciiStr, 8002);


    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randAsciiStr, 8003);
    

    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randAsciiStr, 8004);

    
//     vector<int> strSizeRange = {200};
//   test.kshingle3D(GenSync::SyncProtocol::CPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randSampleTxt);


//    PerformanceData test3 = PerformanceData(tesPts);
//    test3.kshingleBook3D(editDistRange,strSizeRange);
//    test3.genReport("kshingleBookTest3D");
//
//    PerformanceData test2 = PerformanceData(tesPts);
//    test2.kshingleCode3D(editDistRange,strSizeRange);
//    test2.genReport("kshingleTestCode3D");


}

void KshingleSyncPerf::setsofcontent3D() {
    PerformanceData test = PerformanceData(tesPts);
    vector<int> editDistRange;
//   vector<int> strSizeRange = {400, 600, 800, 1000, 1400, 1800};//, 2000, 2200, 2600, 3000};//, 5000, 7000, 9000, 10000};
//
//    test.kshingle3D(GenSync::SyncProtocol::CPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randSampleTxt);


    vector<int> strSizeRange = {100000, 400000, 800000, 1200000};
    editDistRange = {10,1};
    vector<int> lvlRange = {2, 3, 4, 5};
    vector<int> parRange = {2, 4, 6, 8};

    parRange = {10};
//    test.setsofcontent(GenSync::SyncProtocol::IBLTSyncSetDiff, editDistRange, strSizeRange,{4}, parRange, 1, randSampleTxt, 8001);

    test.setsofcontent(GenSync::SyncProtocol::IBLTSyncSetDiff, editDistRange, strSizeRange,{5}, parRange, 100, randSampleTxt, 8001);

    test.setsofcontent(GenSync::SyncProtocol::IBLTSyncSetDiff, editDistRange, strSizeRange,{6}, parRange, 100, randSampleTxt, 8001);

//    test.setsofcontent(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange, 2, 1, randSampleTxt, 8001);
//
//    test.setsofcontent(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange, 3, 1, randSampleTxt, 8001);
//
//    test.setsofcontent(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange, 4, 1, randSampleTxt, 8001);

//    strSizeRange = {2000000};
//    editDistRange = {20000};

//    test.setsofcontent(GenSync::SyncProtocol::IBLTSyncSetDiff, editDistRange, strSizeRange,lvlRange, parRange, 100, randAsciiStr, 8001);

//    test.setsofcontent(GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange,lvlRange, parRange, 1, randAsciiStr, 8001);

//    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange,lvlRange, parRange, 1, randSampleTxt, 8001);


//    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, 2, 1, randSampleTxt, 8001);
//
//    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, 3, 1, randSampleTxt, 8001);
//
//    test.setsofcontent(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, 4, 1, randSampleTxt, 8001);
}

void KshingleSyncPerf::testStrataEst3D() {
    PerformanceData test = PerformanceData(tesPts);
    test.strataEst3D(make_pair(1000,100000), target_confidence);
}
